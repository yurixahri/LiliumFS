#ifndef TAR_H
#define TAR_H

#include <QHttpServerResponder>
#include <QCoreApplication>
#include <QWaitCondition>
#include <QFileInfo>
#include <QIODevice>
#include <QPointer>
#include <QThread>
#include <QMutex>
#include <QDir>
#include <archive.h>
#include <archive_entry.h>
#include "utils/log.h"

class tar_stream;

struct tar_stream_state {
    QByteArray buffer;

    QMutex mutex;
    QWaitCondition can_write;

    bool finished = false;
    bool canceled = false;

    qsizetype max_buffer = 2 * 1024 * 1024;

    // only access this from the Qt/main thread.
    QPointer<tar_stream> stream;
};

class tar_stream : public QIODevice {
public:
    std::shared_ptr<tar_stream_state> state;

    tar_stream(QObject *parent = nullptr) : QIODevice(parent), state(std::make_shared<tar_stream_state>()) {
        state->stream = this;
        open(QIODevice::ReadOnly);
    }

    ~tar_stream() override {
        QMutexLocker locker(&state->mutex);

        state->canceled = true;
        state->can_write.wakeAll();
    }

    bool isSequential() const override {
        return true;
    }

    qint64 bytesAvailable() const override {
        QMutexLocker locker(&state->mutex);
        return state->buffer.size() + QIODevice::bytesAvailable();
    }

    bool atEnd() const override {
        QMutexLocker locker(&state->mutex);
        return state->finished && state->buffer.isEmpty();
    }

    qint64 readData(char *data, qint64 max_len) override {
        QMutexLocker locker(&state->mutex);

        // never block the Qt/server thread.
        if (state->buffer.isEmpty()) return 0;

        qint64 length = qMin<qint64>( max_len, state->buffer.size() );
        memcpy( data, state->buffer.constData(), static_cast<size_t>(length) );
        state->buffer.remove(0, length);
        // wake archive worker if it was blocked by max_buffer.
        state->can_write.wakeAll();

        return length;
    }

    qint64 writeData(const char *, qint64) override{
        // this QIODevice is read-only from Qt's point of view.
        return -1;
    }
};

struct tar_archive_context {
    std::shared_ptr<tar_stream_state> state;
};

static la_ssize_t archiveWriteCallback( archive *, void *client_data, const void *buffer, size_t length) {
    if (length == 0) return 0;

    auto *context = static_cast<tar_archive_context *>(client_data);
    auto state = context->state;
    const char *data = static_cast<const char *>(buffer);
    qint64 written = 0;

    while (written < static_cast<qint64>(length)) {
        bool notify_ready = false;

        QMutexLocker locker(&state->mutex);

        while ( state->buffer.size() >= state->max_buffer && !state->canceled ) {
            state->can_write.wait(&state->mutex);
        }

        if (state->canceled) return -1;

        qsizetype available = state->max_buffer - state->buffer.size();
        qsizetype amount = qMin<qsizetype>( available, static_cast<qsizetype>(length - written) );
        // only need a new readyRead when transitioning from empty -> non-empty.
        notify_ready = state->buffer.isEmpty();
        state->buffer.append( data + written, amount );
        written += amount;

        if (notify_ready) {
            QMetaObject::invokeMethod( qApp, [state]() { if (state->stream) emit state->stream->readyRead(); }, Qt::QueuedConnection );
        }
    }

    return static_cast<la_ssize_t>(length);
}

int addArchiveEntry(archive* a, const QString& relative_path, const QString& absolute_path){
    QFileInfo info(absolute_path);
    if (!info.exists()) {
        // qDebug() << absolute_path;
        return ARCHIVE_FATAL;
    }

    archive_entry* entry = archive_entry_new();
    archive_entry_set_pathname_utf8(entry, relative_path.toUtf8());
    archive_entry_set_size(entry, info.isFile() ? info.size() : 0);

    archive_entry_set_filetype(entry, info.isDir() ? AE_IFDIR : AE_IFREG);
    archive_entry_set_perm(entry, 0644);
    archive_write_header(a, entry);

    if (info.isFile()){
        QFile file(absolute_path);
        if (file.open(QIODevice::ReadOnly)) {
            char buffer[65536];
            while (!file.atEnd()) {
                qint64 len = file.read(buffer, sizeof(buffer));
                if (len > 0) {
                    if (archive_write_data(a, buffer, static_cast<size_t>(len)) < ARCHIVE_OK){
                        archive_entry_free(entry);
                        return ARCHIVE_FATAL;
                    }
                }
            }
        }
    }

    archive_entry_free(entry);

    if (info.isDir()){
        QDir dir(absolute_path);
        QFileInfoList subs = dir.entryInfoList(
                QDir::Files |
                QDir::Dirs |
                QDir::NoDotAndDotDot);

        for (const QFileInfo &sub : subs) {
            QString sub_relative = relative_path + "/" + sub.fileName();
            if (addArchiveEntry(a, sub_relative, sub.absoluteFilePath()) == ARCHIVE_FATAL) return ARCHIVE_FATAL;
        }
    }
    return ARCHIVE_OK;
}

#endif // TAR_H
