#ifndef TAR_H
#define TAR_H

#include <QHttpServerResponder>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <archive.h>
#include <archive_entry.h>
#include "utils/log.h"

static la_ssize_t archiveWriteCallback(archive*, void* client_data, const void* buffer, size_t length){
    if (length == 0) return 0;

    auto responder = static_cast<std::shared_ptr<QHttpServerResponder>*>(client_data);
    QByteArray data(static_cast<const char*>(buffer), static_cast<qsizetype>(length));
    bool canceled = false;

    QMetaObject::invokeMethod(qApp, [responder, data, &canceled]() {
        if ((*responder)->isResponseCanceled()){
            canceled = true;
            return;
        }
        (*responder)->writeChunk(data);
    }, Qt::BlockingQueuedConnection);
    return canceled ? -1 : static_cast<la_ssize_t>(length);
}

int addArchiveEntry(archive* a, const QString& relative_path, const QString& absolute_path){
    QFileInfo info(absolute_path);
    if (!info.exists()) return 0;

    archive_entry* entry = archive_entry_new();
    archive_entry_set_pathname_utf8(entry, relative_path.toUtf8().constData());
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
