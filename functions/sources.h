#ifndef SOURCES_H
#define SOURCES_H

#include <QHttpServerResponse>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "httpHelper.h"
#include "utils/config.h"
#include "fs.h"

bool canAccessSource(const QString &username, QVariant &permission_mode){
    QStringList accounts;
    if (permission_mode.typeId() == QMetaType::QString){
        QString available = permission_mode.toString();
        if (available == "Anyone"){
            return true;
        }else if (available == "Any account"){
            for (const auto &account : config.accounts) {
                accounts.push_back(account.username);
            }
        }else{
            return false;
        }
    }else if(permission_mode.typeId() == QMetaType::QVariantList){
        accounts = permission_mode.toStringList();
    }

    for (const auto &account : accounts) {
        if (account == username)
            return true;
    }
    return false;
}

void removeAccountAccessSource(const QString &username, QVariant &permission_mode){
    if (permission_mode.typeId() == QMetaType::QVariantList){
        QStringList accounts = permission_mode.toStringList();
        accounts.removeIf([&](const QString account){
            return account == username;
        });
        permission_mode = accounts;
    }
}

QHttpServerResponse addSources(const QHttpServerRequest &request){
    QWriteLocker locker(&config.lock);

    if (auto isInvalid = isContentTypeInvalid(request, "application/json")){
        return std::move(isInvalid.value());
    }

    QJsonDocument doc;

    if (auto isInvalid = parseBody(doc, request.body())){
        return std::move(isInvalid.value());
    }

    QJsonObject body = doc.object();
    QJsonArray req_dirs = body["dirs"].toArray();
    QJsonArray req_files = body["files"].toArray();
    static const QRegularExpression invalid(R"([<>:"/\\|?*\x00-\x1F])"); // source name (alias) will work fine with these characters, i just put here just in case i need.

    for (const auto &req_dir: req_dirs){
        bool exist = false;

        for (auto &dir : config.dirs){
            if (req_dir.toObject().value("src").toString() == dir.src){
                exist = true;
                break;
            }
        }
        if (!exist){
            QString name = req_dir.toObject().value("name").toString().replace(invalid, "");
            for (auto &dir : config.dirs){
                if (name == dir.name){
                    name += " (duplicated)";
                    break;
                }
            }

            for (auto &file : config.files){
                if (name == file.name){
                    name += " (duplicated)";
                    break;
                }
            }

            for (auto &vd : config.vds){
                if (name == vd.name){
                    name += " (duplicated)";
                }
            }

            directory_t dir;
            dir.name = name;
            dir.src = req_dir.toObject().value("src").toString();
            dir.can_see = "Anyone";
            dir.can_download = "Anyone";
            dir.can_upload= "No one";
            dir.can_delete = "No one";
            config.dirs.push_back(dir);
        }
    }

    for (const auto &req_file : req_files){
        bool exist = false;

        for (auto &file : config.files){
            if (req_file == file.src){
                exist = true;
                break;
            }
        }
        if (!exist){
            QString name = req_file.toObject().value("name").toString().replace(invalid, "");
            for (auto &dir : config.dirs){
                if (name == dir.name){
                    name += " (duplicated)";
                    break;
                }
            }

            for (auto &file : config.files){
                if (name == file.name){
                    name += " (duplicated)";
                    break;
                }
            }

            for (auto &vd : config.vds){
                if (name == vd.name){
                    name += " (duplicated)";
                }
            }

            file_t file;
            file.name = name;
            file.src = req_file.toObject().value("src").toString();
            file.can_see = "Anyone";
            file.can_download = "Anyone";
            config.files.push_back(file);
        }
    }

    writeConfig();
    return sendStatus(QHttpServerResponse::StatusCode::Ok);
}

QHttpServerResponse changeSource(const QHttpServerRequest &request){
    QWriteLocker locker(&config.lock);

    if (auto isInvalid = isContentTypeInvalid(request, "application/json")){
        return std::move(isInvalid.value());
    }

    QJsonDocument doc;

    if (auto isInvalid = parseBody(doc, request.body())){
        return std::move(isInvalid.value());
    }

    QJsonObject body = doc.object();
    QString type = body["type"].toString();
    if (type != "dir" && type != "file"){
        return sendStatus("Expect type", QHttpServerResponder::StatusCode::BadRequest);
    }


    QString req_name = body.value("name").toString();
    QString req_src = body.value("src").toString();
    // check if name is already exists in source
    for (auto &dir : config.dirs){
        if (req_name == dir.name && req_src != dir.src){
            return sendStatus("Name already exists", QHttpServerResponder::StatusCode::BadRequest);
            break;
        }
    }

    for (auto &file : config.files){
        if (req_name == file.name && req_src != file.src){
            return sendStatus("Name already exists", QHttpServerResponder::StatusCode::BadRequest);
            break;
        }
    }

    for (auto &vd : config.vds){
        if (req_name == vd.name){
            return sendStatus("Name already exists", QHttpServerResponder::StatusCode::BadRequest);
            break;
        }
    }

    if (type == "dir"){
        for (auto &dir : config.dirs){
            if (req_src == dir.src){
                dir.name = req_name;
                dir.can_see = body.value("can_see").toVariant();
                dir.can_download = body.value("can_download").toVariant();
                dir.can_upload= body.value("can_upload").toVariant();
                dir.can_delete = body.value("can_delete").toVariant();
                break;
            }
        }
    }else if (type == "file"){
        for (auto &file : config.files){
            if (req_src == file.src){
                file.name = req_name;
                file.can_see = body.value("can_see").toVariant();
                file.can_download = body.value("can_download").toVariant();
                break;
            }
        }
    }

    writeConfig();
    return sendStatus(QHttpServerResponse::StatusCode::Ok);
}

QHttpServerResponse deleteSource(const QHttpServerRequest &request){
    QWriteLocker locker(&config.lock);

    if (auto isInvalid = isContentTypeInvalid(request, "application/json")){
        return std::move(isInvalid.value());
    }

    QJsonDocument doc;

    if (auto isInvalid = parseBody(doc, request.body())){
        return std::move(isInvalid.value());
    }

    QJsonObject body = doc.object();
    QString type = body["type"].toString();
    if (type != "dir" && type != "file"){
        return sendStatus("Expect type", QHttpServerResponder::StatusCode::BadRequest);
    }

    QString req_src = body.value("src").toString();
    if (type == "dir"){
        config.dirs.removeIf([&](const directory_t &dir){
            return dir.src == req_src;
        });
    }else if (type == "file"){
        config.files.removeIf([&](const file_t &file){
            return file.src == req_src;
        });
    }

    writeConfig();
    return sendStatus(QHttpServerResponse::StatusCode::Ok);
}

QHttpServerResponse getSources(){
    QReadLocker locker(&config.lock);

    QVariantMap root;

    QVariantList dirs;
    for(const auto &d : config.dirs) dirs << dir_to_map(d);
    root["dirs"] = dirs;

    QVariantList files;
    for(const auto &f : config.files) files << file_to_map(f);
    root["files"] = files;

    QVariantList vds;
    for(const auto &vd : config.vds) vds << vd_to_map(vd);
    root["vds"] = vds;

    QJsonDocument doc = QJsonDocument::fromVariant(root);
    QHttpServerResponse response("application/json; charset=utf-8", doc.toJson(QJsonDocument::Compact), QHttpServerResponse::StatusCode::Ok);
    response.setHeaders(createHeaders(.cache = false));
    return response;
}

QHttpServerResponse getDirectoryContents(const QUrl &url){
    QString path =  url.toString();
    if (!isPathExist(path)){
        return sendStatus("Invalid path", QHttpServerResponse::StatusCode::InternalServerError);
    }

    auto dirs = getDirectories(path);
    auto files = getFiles(path);

    QJsonObject list;
    QJsonArray container;

    for (auto &dir : dirs){
        container.append(dir);
    }
    list["dirs"] = container;

    container = QJsonArray();
    for (auto &file : files){
        container.append(file);
    }
    list["files"] = container;
    QJsonDocument doc(list);
    QHttpServerResponse response("application/json; charset=utf-8", doc.toJson(QJsonDocument::Compact), QHttpServerResponse::StatusCode::Ok);
    response.setHeaders(createHeaders(.cache = false));
    return response;
}

QHttpServerResponse getDrives(){
    QStringList drives = getMountedVolumes();
    QJsonArray array;
    for (auto &drive : drives){
        while(drive.endsWith('/')) {
            drive.chop(1);
        }
        array.append(drive);
    }
    QJsonDocument doc(array);
    QHttpServerResponse response("application/json; charset=utf-8", doc.toJson(QJsonDocument::Compact), QHttpServerResponse::StatusCode::Ok);
    response.setHeaders(createHeaders(.cache = false));
    return response;
}

QHttpServerResponse addVirtualDirectory(const QHttpServerRequest &request){
    QWriteLocker locker(&config.lock);

    if (auto isInvalid = isContentTypeInvalid(request, "application/json")){
        return std::move(isInvalid.value());
    }

    QJsonDocument doc;

    if (auto isInvalid = parseBody(doc, request.body())){
        return std::move(isInvalid.value());
    }

    QJsonObject body = doc.object();
    QString name = body["name"].toString();
    static const QRegularExpression invalid(R"([<>:"/\\|?*\x00-\x1F])"); // source name (alias) will work fine with these characters, i just put here just in case i need.

    for (auto &dir : config.dirs){
        if (name == dir.name){
            name += " (duplicated)";
        }
    }

    for (auto &file : config.files){
        if (name == file.name){
            name += " (duplicated)";
        }
    }

    for (auto &vd : config.vds){
        if (name == vd.name){
            name += " (duplicated)";
        }
    }

    vd_t vd;
    vd.name = name;
    vd.can_see = "Anyone";
    vd.can_download = "Anyone";
    config.vds.push_back(vd);

    writeConfig();
    return sendStatus(QHttpServerResponse::StatusCode::Ok);
}

QHttpServerResponse addVirtualChild(const QHttpServerRequest &request){
    QWriteLocker locker(&config.lock);

    if (auto isInvalid = isContentTypeInvalid(request, "application/json")){
        return std::move(isInvalid.value());
    }

    QJsonDocument doc;

    if (auto isInvalid = parseBody(doc, request.body())){
        return std::move(isInvalid.value());
    }

    QJsonObject body = doc.object();
    QString name = body["name"].toString();
    QJsonArray req_dirs = body["dirs"].toArray();
    QJsonArray req_files = body["files"].toArray();
    // QJsonArray dirs = config["dirs"].toArray();
    // QJsonArray files = config["files"].toArray();
    static const QRegularExpression invalid(R"([<>:"/\\|?*\x00-\x1F])"); // source name (alias) will work fine with these characters, i just put here just in case i need.

    for (auto &vd : config.vds){
        if (name == vd.name){
            for (const auto &req_dir : req_dirs){
                bool exist = false;

                for (auto &dir : vd.dirs){
                    if (req_dir.toObject().value("src").toString() == dir.src){
                        exist = true;
                        break;
                    }
                }
                if (!exist){
                    directory_t new_dir;
                    QString name = req_dir.toObject().value("name").toString().replace(invalid, "");
                    for (auto &dir : vd.dirs){
                        if (name == dir.name){
                            name += " (duplicated)";
                            break;
                        }
                    }

                    for (auto &file : vd.files){
                        if (name == file.name){
                            name += " (duplicated)";
                            break;
                        }
                    }

                    new_dir.name = name;
                    new_dir.src = req_dir.toObject().value("src").toString();
                    new_dir.can_see = "Anyone";
                    new_dir.can_download = "Anyone";
                    new_dir.can_upload = "No one";
                    new_dir.can_delete= "No one";

                    vd.dirs.push_back(new_dir);
                }
            }

            for (const auto req_file : req_files){
                bool exist = false;

                for (auto &file : vd.files){
                    if (req_file.toObject().value("src").toString() == file.src){
                        exist = true;
                        break;
                    }
                }
                if (!exist){
                    file_t new_file;
                    QString name = req_file.toObject().value("name").toString().replace(invalid, "");
                    for (auto &dir : vd.dirs){
                        if (name == dir.name){
                            name += " (duplicated)";
                            break;
                        }
                    }

                    for (auto &file : vd.files){
                        if (name == file.name){
                            name += " (duplicated)";
                            break;
                        }
                    }

                    new_file.name = name;
                    new_file.src = req_file.toObject().value("src").toString();
                    new_file.can_see = "Anyone";
                    new_file.can_download = "Anyone";
                    vd.files.push_back(new_file);
                }
            }
            break;
        }
    }

    writeConfig();
    return sendStatus(QHttpServerResponse::StatusCode::Ok);
}

QHttpServerResponse changeVirtualChild(const QHttpServerRequest &request){
    QWriteLocker locker(&config.lock);

    if (auto isInvalid = isContentTypeInvalid(request, "application/json")){
        return std::move(isInvalid.value());
    }

    QJsonDocument doc;

    if (auto isInvalid = parseBody(doc, request.body())){
        return std::move(isInvalid.value());
    }

    QJsonObject body = doc.object();
    QString type = body["type"].toString();
    if (type != "dir" && type != "file"){
        return sendStatus("Expect type", QHttpServerResponder::StatusCode::BadRequest);
    }

    // QJsonArray dirs = config["dirs"].toArray();
    // QJsonArray files = config["files"].toArray();
    for (auto &vd : config.vds){
        if (body["vd_name"].toString() == vd.name){
            for (auto &dir : vd.dirs){
                if (body.value("name").toString() == dir.name && body.value("src").toString() != dir.src){
                    return sendStatus("Name already exists", QHttpServerResponder::StatusCode::BadRequest);
                }
            }

            for (auto &file : vd.files){
                if (body.value("name").toString() == file.name && body.value("src").toString() != file.src){
                    return sendStatus("Name already exists", QHttpServerResponder::StatusCode::BadRequest);
                }
            }

            if (type == "dir"){
                for (auto &dir : vd.dirs){
                    if (body.value("src").toString() == dir.src){
                        dir.name = body.value("name").toString();
                        dir.can_see = body.value("can_see").toVariant();
                        dir.can_download = body.value("can_download").toVariant();
                        dir.can_upload= body.value("can_upload").toVariant();
                        dir.can_delete = body.value("can_delete").toVariant();
                        break;
                    }
                }
            }else if (type == "file"){
                for (auto &file : vd.files){
                    if (body.value("src").toString() == file.src){
                        file.name = body.value("name").toString();
                        file.can_see = body.value("can_see").toVariant();
                        file.can_download = body.value("can_download").toVariant();
                        break;
                    }
                }
            }
            break;
        }
    }

    writeConfig();
    return sendStatus(QHttpServerResponse::StatusCode::Ok);
}

QHttpServerResponse deleteVirtualChild(const QHttpServerRequest &request){
    QWriteLocker locker(&config.lock);

    if (auto isInvalid = isContentTypeInvalid(request, "application/json")){
        return std::move(isInvalid.value());
    }

    QJsonDocument doc;

    if (auto isInvalid = parseBody(doc, request.body())){
        return std::move(isInvalid.value());
    }

    QJsonObject body = doc.object();
    QString type = body["type"].toString();
    if (type != "dir" && type != "file"){
        return sendStatus("Expect type", QHttpServerResponder::StatusCode::BadRequest);
    }

    for (auto &vd : config.vds){
        if (body["vd_name"].toString() == vd.name){
            if (type == "dir"){
                vd.dirs.removeIf([&](const directory_t &dir){
                    return dir.src == body.value("src").toString();
                });
            }else if (type == "file"){
                vd.files.removeIf([&](const file_t &file){
                    return file.src == body.value("src").toString();
                });
            }
        }
    }

    writeConfig();
    return sendStatus(QHttpServerResponse::StatusCode::Ok);
}

QHttpServerResponse changeVirtualDirectory(const QHttpServerRequest &request){
    QWriteLocker locker(&config.lock);

    if (auto isInvalid = isContentTypeInvalid(request, "application/json")){
        return std::move(isInvalid.value());
    }

    QJsonDocument doc;

    if (auto isInvalid = parseBody(doc, request.body())){
        return std::move(isInvalid.value());
    }

    QJsonObject body = doc.object();
    QString old_name = body["old_name"].toString();
    QString new_name = body["new_name"].toString();

    if (old_name != new_name){
        for (const auto &dir : config.dirs){
            if (new_name == dir.name){
                return sendStatus("Name already exists", QHttpServerResponder::StatusCode::BadRequest);
            }
        }

        for (const auto &file : config.files){
            if (new_name == file.name){
                return sendStatus("Name already exists", QHttpServerResponder::StatusCode::BadRequest);
            }
        }

        for (const auto &vd : config.vds){
            if (new_name == vd.name){
                return sendStatus("Name already exists", QHttpServerResponder::StatusCode::BadRequest);
            }
        }
    }

    for (auto &vd : config.vds){
        if (old_name == vd.name){
            vd.name = new_name;
            vd.can_see = body["can_see"].toVariant();
            vd.can_download = body["can_download"].toVariant();
            break;
        }
    }

    writeConfig();
    return sendStatus(QHttpServerResponse::StatusCode::Ok);
}

QHttpServerResponse deleteVirtualDirectory(const QHttpServerRequest &request){
    QWriteLocker locker(&config.lock);

    if (auto isInvalid = isContentTypeInvalid(request, "application/json")){
        return std::move(isInvalid.value());
    }

    QJsonDocument doc;

    if (auto isInvalid = parseBody(doc, request.body())){
        return std::move(isInvalid.value());
    }

    QJsonObject body = doc.object();
    QString name = body["name"].toString();

    config.vds.removeIf([&](const vd_t &vd){
        return vd.name == name;
    });

    writeConfig();
    return sendStatus(QHttpServerResponse::StatusCode::Ok);
}

#endif // SOURCES_H
