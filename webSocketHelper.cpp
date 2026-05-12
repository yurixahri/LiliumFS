#include "webSocketHelper.h"

enum EVENTS{
    SEND,
    UPLOAD_START,
    UPLOAD_COMPLETED,
    UPLOAD_ERROR
};

QHash<QWebSocket*, uploadFile*> uploads;

void emitEvent(QWebSocket *ws, const uint8_t &event, const QJsonValue &data) {
    QJsonObject res;
    switch (event) {
        case EVENTS::SEND :
            ws->sendTextMessage(data.toString());
            break;
        case EVENTS::UPLOAD_START :{
            res["event"] = EVENTS::UPLOAD_START;
            res["data"] = "Ready to upload";
            QJsonDocument doc(res);
            ws->sendTextMessage(doc.toJson(QJsonDocument::Compact));
            break;
            }
        case EVENTS::UPLOAD_ERROR : {
            res["event"] = EVENTS::UPLOAD_ERROR;
            res["data"] = "Could not create file";
            QJsonDocument doc(res);
            ws->sendTextMessage(doc.toJson(QJsonDocument::Compact));
            break;
            }
        default:
            break;
    }
};

void onEvent(QWebSocket *ws, const uint8_t &event, const QJsonValue &data) {
    switch (event) {
        case EVENTS::SEND :
            emitEvent(ws, event, data);
            break;
        case EVENTS::UPLOAD_START :
            uploadStart(ws, data);
            break;
        case EVENTS::UPLOAD_COMPLETED :
            removeUpload(ws);
            break;
        default:
            break;
    }
};

QString makeUniqueName(const QString &path, QString &fileName){
    QString base = QFileInfo(fileName).completeBaseName();
    QString ext  = QFileInfo(fileName).suffix().isEmpty() ? "" : "."+QFileInfo(fileName).suffix();
    uint64_t counter = 1;
    while (isPathExist(path+"/"+fileName)){
        fileName = QString("%1 (%2)%3").arg(base).arg(counter).arg(ext);
        counter++;
    }

    return fileName;
}

void uploadStart(QWebSocket *ws, const QJsonValue &data){
    QReadLocker locker(&config.lock);

    QJsonObject object = data.toObject();
    if (!uploads.contains(ws)){
        bool is_exist = false;
        QString fileName = object.value("name").toString();
        QString relative_path = QUrl::fromPercentEncoding(object.value("path").toString().toUtf8());
        QStringList path = relative_path.split("/");
        for (auto &dir : config.dirs){
            if (path[0] == dir.name){
                path[0] = dir.src;
                is_exist = true;
                break;
            }
        }

        if (!is_exist){
            for (auto &vd : config.vds) {
                if (path[0] == vd.name){
                    for (auto &dir : vd.dirs){
                        if (path[1] == dir.name){
                            path[1] = dir.src;
                            is_exist = true;
                            path.removeFirst();
                            break;
                        }
                    }
                }
                if (is_exist) break;
            }
        }


        if (!is_exist){
            emitEvent(ws, EVENTS::UPLOAD_ERROR);
            return;
        }
        QString absolute_path = path.join("/");

        fileName = makeUniqueName(absolute_path, fileName);

        struct uploadFile *file = new uploadFile();
        file->file.setFileName(absolute_path+"/"+fileName);
        if (!file->file.open(QIODevice::WriteOnly)) {
            emitEvent(ws, EVENTS::UPLOAD_ERROR, NULL);
            delete file;
            return;
        }
        file->expectedSize = object.value("size").toInteger();
        file->receivedSize = 0;
        uploads.insert(ws, file);

        emitEvent(ws, EVENTS::UPLOAD_START);
    }
}

void removeUpload(QWebSocket *ws){
    auto *file = uploads.take(ws);
    if (file) {
        file->file.close();
        if (file->receivedSize < file->expectedSize){
            file->file.remove();
        }
        delete file;
    }
}




