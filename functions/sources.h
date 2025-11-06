#ifndef SOURCES_H
#define SOURCES_H

#include <QHttpServerResponse>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "httpHelper.h"
#include "utils/config.h"
#include "fs.h"

bool canAccessSource(const QString &username, QJsonValueRef permissionMode){
    QJsonArray accounts;
    if (permissionMode.isString()){
        QString available = permissionMode.toString();
        if (available == "Anyone"){
            return true;
        }else if (available == "Any account"){
            accounts = config["accounts"].toArray();
            QJsonArray _accounts;
            for (const auto &account : std::as_const(accounts)) {
                _accounts.append(account.toObject().value("username"));
            }
            accounts =  _accounts;
        }else{
            return false;
        }
    }else if(permissionMode.isArray()){
        accounts = permissionMode.toArray();
    }

    for (const auto &account : std::as_const(accounts)) {
        if (account.toString() == username )
            return true;
    }
    return false;
}

QJsonArray removeAccountAccessSource(const QString &username, QJsonValueRef permissionMode){
    QJsonArray oldAccounts = permissionMode.toArray();
    QJsonArray newAccounts;

    for (const auto &account : std::as_const(oldAccounts)) {
        if (account.toString() != username )
           newAccounts.append(account);
    }

    return  newAccounts;
}

QHttpServerResponse addSources(const QHttpServerRequest &request){
    if (auto isInvalid = isContentTypeInvalid(request, "application/json")){
        return std::move(isInvalid.value());
    }

    QJsonDocument doc;

    if (auto isInvalid = parseBody(doc, request.body())){
        return std::move(isInvalid.value());
    }

    QJsonObject body = doc.object();
    QJsonArray folders = body["folders"].toArray();
    QJsonArray files = body["files"].toArray();
    QJsonArray sourceFolders = config["folders"].toArray();
    QJsonArray sourceFiles = config["files"].toArray();
    static const QRegularExpression invalid(R"([<>:"/\\|?*\x00-\x1F])"); // source name (alias) will work fine with these characters, i just put here just in case i need.

    for (const auto &folder : std::as_const(folders)){
        bool exist = false;

        for (const auto &sourceFolder : std::as_const(sourceFolders)){
            if (folder.toObject().value("src").toString() == sourceFolder.toObject().value("src").toString()){
                exist = true;
                break;
            }
        }
        if (!exist){
            auto _folder = folder.toObject();
            QString name = _folder.value("name").toString().replace(invalid, "");
            for (const auto &sourceFolder : std::as_const(sourceFolders)){
                if (name == sourceFolder.toObject().value("name").toString()){
                    name += " (duplicated)";
                    break;
                }
            }

            for (const auto &sourceFile : std::as_const(sourceFiles)){
                if (name == sourceFile.toObject().value("name").toString()){
                    name += " (duplicated)";
                    break;
                }
            }

            _folder["name"] = name;
            _folder["canSee"] = "Anyone";
            _folder["canUpload"] = "No one";
            _folder["canDelete"] = "No one";
            _folder["canDownload"] = "Anyone";
            sourceFolders.push_back(_folder);
        }
    }

    for (const auto file : std::as_const(files)){
        bool exist = false;

        for (const auto &sourceFile : std::as_const(sourceFiles)){
            if (file.toObject().value("src").toString() == sourceFile.toObject().value("src").toString()){
                exist = true;
                break;
            }
        }
        if (!exist){
            auto _file = file.toObject();
            QString name = _file["name"].toString().replace(invalid, "");
            for (const auto &sourceFolder : std::as_const(sourceFolders)){
                if (name == sourceFolder.toObject().value("name").toString()){
                    name += " (duplicated)";
                    break;
                }
            }

            for (const auto &sourceFile : std::as_const(sourceFiles)){
                if (name == sourceFile.toObject().value("name").toString()){
                    name += " (duplicated)";
                    break;
                }
            }
            _file["name"] = name;
            _file["canSee"] = "Anyone";
            _file["canDownload"] = "Anyone";
            sourceFiles.push_back(_file);
        }
    }

    config["folders"] = sourceFolders;
    config["files"] = sourceFiles;

    writeConfig();
    return sendStatus(QHttpServerResponse::StatusCode::Ok);
}

QHttpServerResponse changeSource(const QHttpServerRequest &request){
    if (auto isInvalid = isContentTypeInvalid(request, "application/json")){
        return std::move(isInvalid.value());
    }

    QJsonDocument doc;

    if (auto isInvalid = parseBody(doc, request.body())){
        return std::move(isInvalid.value());
    }

    QJsonObject body = doc.object();
    QString type = body["type"].toString();
    if (type != "folder" && type != "file"){
        return sendStatus("Expect type", QHttpServerResponder::StatusCode::BadRequest);
    }


    QJsonArray sourceFolders = config["folders"].toArray();
    QJsonArray sourceFiles = config["files"].toArray();
    // check if name is already exists in source
    for (const auto &folder : std::as_const(sourceFolders)){
        if (body.value("name").toString() == folder.toObject().value("name").toString() && body.value("src").toString() != folder.toObject().value("src").toString()){
            return sendStatus("Name already exists", QHttpServerResponder::StatusCode::BadRequest);
            break;
        }
    }

    for (const auto &file : std::as_const(sourceFiles)){
        if (body.value("name").toString() == file.toObject().value("name").toString() && body.value("src").toString() != file.toObject().value("src").toString()){
            return sendStatus("Name already exists", QHttpServerResponder::StatusCode::BadRequest);
            break;
        }
    }

    // add to source
    uint32_t index = 0;
    if (type == "folder"){
        for (const auto &folder : std::as_const(sourceFolders)){
            if (body.value("src").toString() == folder.toObject().value("src").toString()){
                body.remove("type");
                sourceFolders[index] = body;
                break;
            }
            ++index;
        }
        config["folders"] = sourceFolders;
    }else if (type == "file"){
        for (const auto &file : std::as_const(sourceFiles)){
            if (body.value("src").toString() == file.toObject().value("src").toString()){
                body.remove("type");
                sourceFiles[index] = body;
                break;
            }
            ++index;
        }
        config["files"] = sourceFiles;
    }

    writeConfig();
    return sendStatus(QHttpServerResponse::StatusCode::Ok);
}

QHttpServerResponse deleteSource(const QHttpServerRequest &request){
    if (auto isInvalid = isContentTypeInvalid(request, "application/json")){
        return std::move(isInvalid.value());
    }

    QJsonDocument doc;

    if (auto isInvalid = parseBody(doc, request.body())){
        return std::move(isInvalid.value());
    }

    QJsonObject body = doc.object();
    QString type = body["type"].toString();
    if (type != "folder" && type != "file"){
        return sendStatus("Expect type", QHttpServerResponder::StatusCode::BadRequest);
    }


    if (type == "folder"){
        QJsonArray oldFolders = config["folders"].toArray();
        QJsonArray newFolders;
        for (const auto &folder : std::as_const(oldFolders)){
            if (body.value("src").toString() != folder.toObject().value("src").toString()){
                newFolders.append(folder);
            }
        }
        config["folders"] = newFolders;
    }else if (type == "file"){
        QJsonArray oldFiles = config["files"].toArray();
        QJsonArray newFiles;
        for (const auto &file : std::as_const(oldFiles)){
            if (body.value("src").toString() != file.toObject().value("src").toString()){
                newFiles.append(file);
            }
        }
        config["files"] = newFiles;
    }

    writeConfig();
    return sendStatus(QHttpServerResponse::StatusCode::Ok);
}

QHttpServerResponse getSources(){
    QJsonObject sources;
    sources["folders"] = config["folders"];
    sources["files"] = config["files"];
    sources["virtual"] = config["virtual"];
    QJsonDocument doc(sources);
    QHttpServerResponse response("application/json; charset=utf-8", doc.toJson(QJsonDocument::Compact), QHttpServerResponse::StatusCode::Ok);
    response.setHeaders(createHeaders(.cache = false));
    return response;
}

QHttpServerResponse getDirectoryContents(const QUrl &url){
    QString path =  url.toString();
    if (!isExist(path)){
        return sendStatus("Invalid path", QHttpServerResponse::StatusCode::InternalServerError);
    }

    auto folders = getDirectories(path);
    auto files = getFiles(path);

    QJsonObject list;
    QJsonArray container;

    for (auto &folder : folders){
        container.append(folder);
    }
    list["folders"] = container;

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

QHttpServerResponse _getDrives(){
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

QHttpServerResponse addVirtualRoot(const QHttpServerRequest &request){
    if (auto isInvalid = isContentTypeInvalid(request, "application/json")){
        return std::move(isInvalid.value());
    }

    QJsonDocument doc;

    if (auto isInvalid = parseBody(doc, request.body())){
        return std::move(isInvalid.value());
    }

    QJsonObject body = doc.object();
    QString name = body["name"].toString();
    QJsonArray sourceFolders = config["folders"].toArray();
    QJsonArray sourceFiles = config["files"].toArray();
    QJsonArray sourceVirtualRoots = config["virtual"].toArray();
    static const QRegularExpression invalid(R"([<>:"/\\|?*\x00-\x1F])"); // source name (alias) will work fine with these characters, i just put here just in case i need.

    for (const auto &sourceFolder : std::as_const(sourceFolders)){
        if (name == sourceFolder.toObject().value("name").toString()){
            name += " (duplicated)";
        }
    }

    for (const auto &sourceFile : std::as_const(sourceFiles)){
        if (name == sourceFile.toObject().value("name").toString()){
            name += " (duplicated)";
        }
    }

    for (const auto &virtualRoot : std::as_const(sourceVirtualRoots)){
        if (name == virtualRoot.toObject().value("name").toString()){
            name += " (duplicated)";
        }
    }

    QJsonObject virtualRoot;
    virtualRoot["name"] = name;
    virtualRoot["folders"] = QJsonArray();
    virtualRoot["files"] = QJsonArray();
    virtualRoot["canSee"] = "Anyone";
    virtualRoot["canDownload"] = "Anyone";

    sourceVirtualRoots.append(virtualRoot);
    config["virtual"] = sourceVirtualRoots;
    writeConfig();
    return sendStatus(QHttpServerResponse::StatusCode::Ok);
}

QHttpServerResponse addVirtualChild(const QHttpServerRequest &request){
    if (auto isInvalid = isContentTypeInvalid(request, "application/json")){
        return std::move(isInvalid.value());
    }

    QJsonDocument doc;

    if (auto isInvalid = parseBody(doc, request.body())){
        return std::move(isInvalid.value());
    }

    QJsonObject body = doc.object();
    QString name = body["name"].toString();
    QJsonArray folders = body["folders"].toArray();
    QJsonArray files = body["files"].toArray();
    // QJsonArray sourceFolders = config["folders"].toArray();
    // QJsonArray sourceFiles = config["files"].toArray();
    QJsonArray sourceVirtualRoots = config["virtual"].toArray();
    static const QRegularExpression invalid(R"([<>:"/\\|?*\x00-\x1F])"); // source name (alias) will work fine with these characters, i just put here just in case i need.

    uint32_t index= 0;
    for (const auto &virtualRoot : std::as_const(sourceVirtualRoots)){
        QJsonObject _virtualRoot = virtualRoot.toObject();
        if (name == _virtualRoot.value("name").toString()){
            QJsonArray virtualFolders = _virtualRoot.value("folders").toArray();
            QJsonArray virtualFiles = _virtualRoot.value("files").toArray();

            for (const auto &folder : std::as_const(folders)){
                bool exist = false;

                for (const auto &virtualFolder : std::as_const(virtualFolders)){
                    if (folder.toObject().value("src").toString() == virtualFolder.toObject().value("src").toString()){
                        exist = true;
                        break;
                    }
                }
                if (!exist){
                    auto _folder = folder.toObject();
                    QString name = _folder.value("name").toString().replace(invalid, "");
                    for (const auto &virtualFolder : std::as_const(virtualFolders)){
                        if (name == virtualFolder.toObject().value("name").toString()){
                            name += " (duplicated)";
                            break;
                        }
                    }

                    for (const auto &virtualFile : std::as_const(virtualFiles)){
                        if (name == virtualFile.toObject().value("name").toString()){
                            name += " (duplicated)";
                            break;
                        }
                    }

                    _folder["name"] = name;
                    _folder["canSee"] = "Anyone";
                    _folder["canUpload"] = "No one";
                    _folder["canDelete"] = "No one";
                    _folder["canDownload"] = "Anyone";
                    virtualFolders.push_back(_folder);
                }
            }

            for (const auto file : std::as_const(files)){
                bool exist = false;

                for (const auto &virtualFile : std::as_const(virtualFiles)){
                    if (file.toObject().value("src").toString() == virtualFile.toObject().value("src").toString()){
                        exist = true;
                        break;
                    }
                }
                if (!exist){
                    auto _file = file.toObject();
                    QString name = _file["name"].toString().replace(invalid, "");
                    for (const auto &virtualFolder : std::as_const(virtualFolders)){
                        if (name == virtualFolder.toObject().value("name").toString()){
                            name += " (duplicated)";
                            break;
                        }
                    }

                    for (const auto &virtualFile : std::as_const(virtualFiles)){
                        if (name == virtualFile.toObject().value("name").toString()){
                            name += " (duplicated)";
                            break;
                        }
                    }
                    _file["name"] = name;
                    _file["canSee"] = "Anyone";
                    _file["canDownload"] = "Anyone";
                    virtualFiles.push_back(_file);
                }
            }

            _virtualRoot["folders"] = virtualFolders;
            _virtualRoot["files"] = virtualFiles;
            sourceVirtualRoots[index] = _virtualRoot;
            break;
        }
        ++index;
    }

    config["virtual"] = sourceVirtualRoots;
    writeConfig();
    return sendStatus(QHttpServerResponse::StatusCode::Ok);
}

QHttpServerResponse changeVirtualChild(const QHttpServerRequest &request){
    if (auto isInvalid = isContentTypeInvalid(request, "application/json")){
        return std::move(isInvalid.value());
    }

    QJsonDocument doc;

    if (auto isInvalid = parseBody(doc, request.body())){
        return std::move(isInvalid.value());
    }

    QJsonObject body = doc.object();
    QString type = body["type"].toString();
    if (type != "folder" && type != "file"){
        return sendStatus("Expect type", QHttpServerResponder::StatusCode::BadRequest);
    }


    // QJsonArray sourceFolders = config["folders"].toArray();
    // QJsonArray sourceFiles = config["files"].toArray();

    QJsonArray sourceVirtualRoots = config["virtual"].toArray();
    uint32_t index= 0;
    for (const auto &virtualRoot : std::as_const(sourceVirtualRoots)){
        QJsonObject _virtualRoot = virtualRoot.toObject();
        if (body["virtualRootName"].toString() == _virtualRoot.value("name").toString()){
            QJsonArray virtualFolders = _virtualRoot.value("folders").toArray();
            QJsonArray virtualFiles = _virtualRoot.value("files").toArray();

            for (const auto &folder : std::as_const(virtualFolders)){
                if (body.value("name").toString() == folder.toObject().value("name").toString() && body.value("src").toString() != folder.toObject().value("src").toString()){
                    return sendStatus("Name already exists", QHttpServerResponder::StatusCode::BadRequest);
                }
            }

            for (const auto &file : std::as_const(virtualFiles)){
                if (body.value("name").toString() == file.toObject().value("name").toString() && body.value("src").toString() != file.toObject().value("src").toString()){
                    return sendStatus("Name already exists", QHttpServerResponder::StatusCode::BadRequest);
                }
            }

            body.remove("virtualRootName");
            uint32_t indexChild = 0;
            if (type == "folder"){
                for (const auto &folder : std::as_const(virtualFolders)){
                    if (body.value("src").toString() == folder.toObject().value("src").toString()){
                        body.remove("type");
                        virtualFolders[indexChild] = body;
                        break;
                    }
                    ++indexChild;
                }
                _virtualRoot["folders"] = virtualFolders;
            }else if (type == "file"){
                for (const auto &file : std::as_const(virtualFiles)){
                    if (body.value("src").toString() == file.toObject().value("src").toString()){
                        body.remove("type");
                        virtualFiles[indexChild] = body;
                        break;
                    }
                    ++indexChild;
                }
                _virtualRoot["files"] = virtualFiles;
            }

            sourceVirtualRoots[index] = _virtualRoot;
            break;
        }
        ++index;
    }

    config["virtual"] = sourceVirtualRoots;
    writeConfig();
    return sendStatus(QHttpServerResponse::StatusCode::Ok);
}

QHttpServerResponse deleteVirtualChild(const QHttpServerRequest &request){
    if (auto isInvalid = isContentTypeInvalid(request, "application/json")){
        return std::move(isInvalid.value());
    }

    QJsonDocument doc;

    if (auto isInvalid = parseBody(doc, request.body())){
        return std::move(isInvalid.value());
    }

    QJsonObject body = doc.object();
    QString type = body["type"].toString();
    if (type != "folder" && type != "file"){
        return sendStatus("Expect type", QHttpServerResponder::StatusCode::BadRequest);
    }

    QJsonArray sourceVirtualRoots = config["virtual"].toArray();
    uint32_t index= 0;
    for (const auto &virtualRoot : std::as_const(sourceVirtualRoots)){
        QJsonObject _virtualRoot = virtualRoot.toObject();
        if (body["virtualRootName"].toString() == _virtualRoot.value("name").toString()){

            if (type == "folder"){
                QJsonArray oldVirtualFolders = _virtualRoot.value("folders").toArray();
                QJsonArray newVirtualFolders;
                for (const auto &folder : std::as_const(oldVirtualFolders)){
                    if (body.value("src").toString() != folder.toObject().value("src").toString()){
                        newVirtualFolders.append(folder);
                    }
                }
                _virtualRoot["folders"] = newVirtualFolders;
            }else if (type == "file"){
                QJsonArray oldVirtualFiles = _virtualRoot.value("files").toArray();
                QJsonArray newVirtualFiles;
                for (const auto &file : std::as_const(oldVirtualFiles)){
                    if (body.value("src").toString() != file.toObject().value("src").toString()){
                        newVirtualFiles.append(file);
                    }
                }
                _virtualRoot["files"] = newVirtualFiles;
            }

            sourceVirtualRoots[index] = _virtualRoot;
        }
        ++index;
    }

    config["virtual"] = sourceVirtualRoots;
    writeConfig();
    return sendStatus(QHttpServerResponse::StatusCode::Ok);
}

QHttpServerResponse changeVirtualRoot(const QHttpServerRequest &request){
    if (auto isInvalid = isContentTypeInvalid(request, "application/json")){
        return std::move(isInvalid.value());
    }

    QJsonDocument doc;

    if (auto isInvalid = parseBody(doc, request.body())){
        return std::move(isInvalid.value());
    }

    QJsonObject body = doc.object();
    QString oldName = body["oldName"].toString();
    QString newName = body["newName"].toString();
    QJsonArray sourceFolders = config["folders"].toArray();
    QJsonArray sourceFiles = config["files"].toArray();
    QJsonArray sourceVirtualRoots = config["virtual"].toArray();

    if (oldName != newName){
        for (const auto &sourceFolder : std::as_const(sourceFolders)){
            if (newName == sourceFolder.toObject().value("name").toString()){
                return sendStatus("Name already exists", QHttpServerResponder::StatusCode::BadRequest);
            }
        }

        for (const auto &sourceFile : std::as_const(sourceFiles)){
            if (newName == sourceFile.toObject().value("name").toString()){
                return sendStatus("Name already exists", QHttpServerResponder::StatusCode::BadRequest);
            }
        }

        for (const auto &virtualRoot : std::as_const(sourceVirtualRoots)){
            if (newName == virtualRoot.toObject().value("name").toString()){
                return sendStatus("Name already exists", QHttpServerResponder::StatusCode::BadRequest);
            }
        }
    }

    uint32_t index= 0;
    for (const auto &virtualRoot : std::as_const(sourceVirtualRoots)){
        QJsonObject _virtualRoot = virtualRoot.toObject();
        if (oldName == _virtualRoot.value("name").toString()){
            _virtualRoot["name"] = newName;
            _virtualRoot["canSee"] = body["canSee"];
            _virtualRoot["canDownload"] = body["canDownload"];

            sourceVirtualRoots[index] = _virtualRoot;
            break;
        }
        ++index;
    }

    config["virtual"] = sourceVirtualRoots;
    writeConfig();
    return sendStatus(QHttpServerResponse::StatusCode::Ok);
}

QHttpServerResponse deleteVirtualRoot(const QHttpServerRequest &request){
    if (auto isInvalid = isContentTypeInvalid(request, "application/json")){
        return std::move(isInvalid.value());
    }

    QJsonDocument doc;

    if (auto isInvalid = parseBody(doc, request.body())){
        return std::move(isInvalid.value());
    }

    QJsonObject body = doc.object();
    QString name = body["name"].toString();

    QJsonArray oldSourceVirtualRoots = config["virtual"].toArray();
    QJsonArray newSourceVirtualRoots;

    for (const auto &virtualRoot : std::as_const(oldSourceVirtualRoots)){
        QJsonObject _virtualRoot = virtualRoot.toObject();
        if (name != _virtualRoot.value("name").toString()){
            newSourceVirtualRoots.push_back(_virtualRoot);
        }
    }

    config["virtual"] = newSourceVirtualRoots;
    writeConfig();
    return sendStatus(QHttpServerResponse::StatusCode::Ok);
}

#endif // SOURCES_H
