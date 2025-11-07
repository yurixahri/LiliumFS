#include <QCoreApplication>
#include <QHttpServer>
#include <QHttpServerResponse>
#include <QHttpServerRouterRule>
#include <QTcpServer>
#include <QMimeDatabase>
#include <QMimeType>

#include "utils/config.h"
#include "utils/stringUtils.h"
#include "utils/auth.h"
#include "fs.h"
#include "httpHelper.h"
#include "webSocketHelper.h"
#include "functions/accounts.h"
#include "functions/sources.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (!getConfig()) return 1;

    const uint16_t port = config["port"].toInt();

    QHttpServer server;
    QTcpServer tcpServer;

    /* manage sources section */
    server.route("/__/admin/api/getDrives", [](const QHttpServerRequest &request) {
        if (!request.remoteAddress().isLoopback() && !isAuthorized(request)) {
            return sendStatus("Forbidden", QHttpServerResponse::StatusCode::Forbidden);
        }

        /*
            I was struggling to choose with style of frontend I want to use, I end up using svelte.
            You can custom endpoint yourself to send htmx component, or the old MVC way with inja template

            htmx version:
                QString html = "<ul>";
                for (auto &drive : drives) {
                    html += "<li>" + drive.toHtmlEscaped() + "</li>";
                }
                html += "</ul>";
                QHttpServerResponse response("text/html; charset=utf-8", html, QHttpServerResponse::StatusCode::Ok);
            mvc version:
                QString html = mainPage("Hello"); //see example in htmlTemplate.h
                QHttpServerResponse response("text/html; charset=utf-8", html, QHttpServerResponse::StatusCode::Ok);

            see the httpTemplate.h for reference
        */
        return _getDrives();
    });

    server.route("/__/admin/api/getDirectoryContents/*", [](const QUrl &url, const QHttpServerRequest &request) {
        if (!request.remoteAddress().isLoopback() && !isAuthorized(request)) {
            return sendStatus("Forbidden", QHttpServerResponse::StatusCode::Forbidden);
        }


       return getDirectoryContents(url);
    });

    server.route("/__/admin/api/getSources", [](const QHttpServerRequest &request) {
        if (!request.remoteAddress().isLoopback() && !isAuthorized(request)) {
            return sendStatus("Forbidden", QHttpServerResponse::StatusCode::Forbidden);
        }

        return getSources();
    });

    server.route("/__/admin/api/addSources", QHttpServerRequest::Method::Options,
        [](const QHttpServerRequest &) {
        return sendStatus(QHttpServerResponse::StatusCode::Ok);
        });

    server.route("/__/admin/api/addSources", QHttpServerRequest::Method::Post, [](const QHttpServerRequest &request) {
        if (!request.remoteAddress().isLoopback() && !isAuthorized(request)) {
            return sendStatus("Forbidden", QHttpServerResponse::StatusCode::Forbidden);
        }
        
        return addSources(request);
    });

    server.route("/__/admin/api/changeSource", QHttpServerRequest::Method::Options,
        [](const QHttpServerRequest &) {
         return sendStatus(QHttpServerResponse::StatusCode::Ok);
        });

    server.route("/__/admin/api/changeSource", QHttpServerRequest::Method::Post, [](const QHttpServerRequest &request) {
        if (!request.remoteAddress().isLoopback() && !isAuthorized(request)) {
            return sendStatus("Forbidden", QHttpServerResponse::StatusCode::Forbidden);
        }

        return changeSource(request);
    });

    server.route("/__/admin/api/deleteSource", QHttpServerRequest::Method::Options,
        [](const QHttpServerRequest &) {
            return sendStatus(QHttpServerResponse::StatusCode::Ok);
        });

    server.route("/__/admin/api/deleteSource", QHttpServerRequest::Method::Post, [](const QHttpServerRequest &request) {
        if (!request.remoteAddress().isLoopback() && !isAuthorized(request)) {
            return sendStatus("Forbidden", QHttpServerResponse::StatusCode::Forbidden);
        }

        return deleteSource(request);
    });

    server.route("/__/admin/api/addVirtualRoot", QHttpServerRequest::Method::Options,
                 [](const QHttpServerRequest &) {
                     return sendStatus(QHttpServerResponse::StatusCode::Ok);
                 });

    server.route("/__/admin/api/addVirtualRoot", QHttpServerRequest::Method::Post, [](const QHttpServerRequest &request) {
        if (!request.remoteAddress().isLoopback() && !isAuthorized(request)) {
            return sendStatus("Forbidden", QHttpServerResponse::StatusCode::Forbidden);
        }

        return addVirtualRoot(request);
    });

    server.route("/__/admin/api/changeVirtualRoot", QHttpServerRequest::Method::Options,
                 [](const QHttpServerRequest &) {
                     return sendStatus(QHttpServerResponse::StatusCode::Ok);
                 });

    server.route("/__/admin/api/changeVirtualRoot", QHttpServerRequest::Method::Post, [](const QHttpServerRequest &request) {
        if (!request.remoteAddress().isLoopback() && !isAuthorized(request)) {
            return sendStatus("Forbidden", QHttpServerResponse::StatusCode::Forbidden);
        }

        return changeVirtualRoot(request);
    });

    server.route("/__/admin/api/deleteVirtualRoot", QHttpServerRequest::Method::Options,
                 [](const QHttpServerRequest &) {
                     return sendStatus(QHttpServerResponse::StatusCode::Ok);
                 });

    server.route("/__/admin/api/deleteVirtualRoot", QHttpServerRequest::Method::Post, [](const QHttpServerRequest &request) {
        if (!request.remoteAddress().isLoopback() && !isAuthorized(request)) {
            return sendStatus("Forbidden", QHttpServerResponse::StatusCode::Forbidden);
        }

        return deleteVirtualRoot(request);
    });

    server.route("/__/admin/api/addVirtualChild", QHttpServerRequest::Method::Options,
                 [](const QHttpServerRequest &) {
                     return sendStatus(QHttpServerResponse::StatusCode::Ok);
                 });

    server.route("/__/admin/api/addVirtualChild", QHttpServerRequest::Method::Post, [](const QHttpServerRequest &request) {
        if (!request.remoteAddress().isLoopback() && !isAuthorized(request)) {
            return sendStatus("Forbidden", QHttpServerResponse::StatusCode::Forbidden);
        }

        return addVirtualChild(request);
    });

    server.route("/__/admin/api/changeVirtualChild", QHttpServerRequest::Method::Options,
                 [](const QHttpServerRequest &) {
                     return sendStatus(QHttpServerResponse::StatusCode::Ok);
                 });

    server.route("/__/admin/api/changeVirtualChild", QHttpServerRequest::Method::Post, [](const QHttpServerRequest &request) {
        if (!request.remoteAddress().isLoopback() && !isAuthorized(request)) {
            return sendStatus("Forbidden", QHttpServerResponse::StatusCode::Forbidden);
        }

        return changeVirtualChild(request);
    });

    server.route("/__/admin/api/deleteVirtualChild", QHttpServerRequest::Method::Options,
                 [](const QHttpServerRequest &) {
                     return sendStatus(QHttpServerResponse::StatusCode::Ok);
                 });

    server.route("/__/admin/api/deleteVirtualChild", QHttpServerRequest::Method::Post, [](const QHttpServerRequest &request) {
        if (!request.remoteAddress().isLoopback() && !isAuthorized(request)) {
            return sendStatus("Forbidden", QHttpServerResponse::StatusCode::Forbidden);
        }

        return deleteVirtualChild(request);
    });

    /* manage settings section */
    server.route("/__/admin/api/getSettings", [](const QHttpServerRequest &request) {
        if (!request.remoteAddress().isLoopback() && !isAuthorized(request)) {
            return sendStatus("Forbidden", QHttpServerResponse::StatusCode::Forbidden);
        }

        QJsonObject settings;
        settings["port"] = config["port"];

        QJsonDocument doc(settings);
        QHttpServerResponse response("application/json; charset=utf-8", doc.toJson(QJsonDocument::Compact), QHttpServerResponse::StatusCode::Ok);

        response.setHeaders(createHeaders(.cache = false));
        return response;
    });

    server.route("/__/admin/api/changeSettings", QHttpServerRequest::Method::Options,
                 [](const QHttpServerRequest &) {
                     return sendStatus(QHttpServerResponse::StatusCode::Ok);
                 });

    server.route("/__/admin/api/changeSettings", QHttpServerRequest::Method::Post, [&tcpServer, &server](const QHttpServerRequest &request) {
        if (!request.remoteAddress().isLoopback() && !isAuthorized(request)) {
            return sendStatus("Forbidden", QHttpServerResponse::StatusCode::Forbidden);
        }

        if (auto isInvalid = isContentTypeInvalid(request, "application/json")){
            return std::move(isInvalid.value());
        }

        QJsonDocument doc;

        if (auto isInvalid = parseBody(doc, request.body())){
            return std::move(isInvalid.value());
        }

        QJsonObject body = doc.object();
        config["port"] = body["port"].toInt();
        writeConfig();

        if (!tcpReload(tcpServer, server,body["port"].toInt())){
            return sendStatus("Could not change server port!", QHttpServerResponder::StatusCode::InternalServerError);
        }


        return sendStatus(QHttpServerResponse::StatusCode::Ok);
    });

    /* manage accounts section */

    server.route("/__/admin/api/getAccounts", [](const QHttpServerRequest &request) {
        if (!request.remoteAddress().isLoopback() && !isAuthorized(request)) {
            return sendStatus("Forbidden", QHttpServerResponse::StatusCode::Forbidden);
        }

        return getAccounts();
    });

    server.route("/__/admin/api/addAccount", QHttpServerRequest::Method::Options,
                 [](const QHttpServerRequest &) {
                     return sendStatus(QHttpServerResponse::StatusCode::Ok);
                 });

    server.route("/__/admin/api/addAccount", QHttpServerRequest::Method::Post, [](const QHttpServerRequest &request) {
        if (!request.remoteAddress().isLoopback() && !isAuthorized(request)) {
            return sendStatus("Forbidden", QHttpServerResponse::StatusCode::Forbidden);
        }

        return addAccount(request);
    });

    server.route("/__/admin/api/deleteAccount", QHttpServerRequest::Method::Options,
                 [](const QHttpServerRequest &) {
                     return sendStatus(QHttpServerResponse::StatusCode::Ok);
                 });

    server.route("/__/admin/api/deleteAccount", QHttpServerRequest::Method::Post, [](const QHttpServerRequest &request) {
        if (!request.remoteAddress().isLoopback() && !isAuthorized(request)) {
            return sendStatus("Forbidden", QHttpServerResponse::StatusCode::Forbidden);
        }

        return deleteAccount(request);
    });

    server.route("/__/admin/api/changeAccount", QHttpServerRequest::Method::Options,
                 [](const QHttpServerRequest &) {
                     return sendStatus(QHttpServerResponse::StatusCode::Ok);
                 });

    server.route("/__/admin/api/changeAccount", QHttpServerRequest::Method::Post, [](const QHttpServerRequest &request) {
        if (!request.remoteAddress().isLoopback() && !isAuthorized(request)) {
            return sendStatus("Forbidden", QHttpServerResponse::StatusCode::Forbidden);
        }

        return changeAccount(request);
    });

    // checking first time initialize
    server.route("/__/admin/api/init", [](const QHttpServerRequest &request) {
        if (!request.remoteAddress().isLoopback() && !isAuthorized(request)) {
            return sendStatus("Forbidden", QHttpServerResponse::StatusCode::Forbidden);
        }

        if (config["accounts"].toArray().size() == 0){
            return sendStatus(QHttpServerResponse::StatusCode::Ok);
        }else{
            return sendStatus(QHttpServerResponse::StatusCode::ServiceUnavailable);
        }
    });

    server.route("/__/admin/*", [](const QUrl &url, const QHttpServerRequest &request) {
        // if (!request.remoteAddress().isLoopback() && !isAuthorized(request)) {
        //     return sendStatus("Forbidden", QHttpServerResponse::StatusCode::Forbidden);
        // }

        QString path = url.toString();
        while(path.endsWith('/')) {
            path.chop(1);
        }

        //logNormal(path.toStdString());
        QString root = "./client/__/admin/";
        if (!isExist( root+path)) {
            return sendStatus("This path is not exist", QHttpServerResponder::StatusCode::BadRequest);
        }

        if (!isFile(root+path)){
            if (!path.isEmpty()) path += "/";
            if(isExist(root+path+"index.html")){
                QFile file(root+path+"index.html");
                file.open(QIODevice::ReadOnly);
                QByteArray fileContent = file.readAll();
                file.close();
                QMimeType mime = getMimeType(root+path+"index.html");
                QHttpServerResponse response(mime.name().toUtf8(), fileContent);
                response.setHeaders(createHeaders(.cache = false));
                return response;
            }else{
                return sendStatus("No page here", QHttpServerResponder::StatusCode::BadRequest);
            }
        }else{
            QFile file(root+path);
            file.open(QIODevice::ReadOnly);
            QByteArray fileContent = file.readAll();
            file.close();
            QMimeType mime = getMimeType(root+path);
            QHttpServerResponse response(fileContent, mime.name().toUtf8());
            QHttpHeaders headers = createHeaders(.cache = false);
            headers.append("Content-Type", mime.name().toUtf8());
            response.setHeaders(headers);
            return response;
        }
    });

    // For svelte static files
    server.route("/_app/*", [](const QUrl &url) {
        QString path = url.toString();
        while(path.endsWith('/')) {
            path.chop(1);
        }

        QString root = "./client/_app/";
        if (!isExist(root+path)) {
            return sendStatus("This path is not exist", QHttpServerResponder::StatusCode::BadRequest);
        }

        if (isFile(root+path)){
            QFile file(root+path);
            file.open(QIODevice::ReadOnly);
            QByteArray fileContent = file.readAll();
            file.close();
            QMimeType mime = getMimeType(root+path);
            QHttpServerResponse response(fileContent);
            QHttpHeaders headers = createHeaders(.cache = false);
            headers.append("Content-Type", mime.name().toUtf8());
            response.setHeaders(headers);
            return response;
        }
    });

    /* user api */
    server.route("/__/api/getSources/*", [](const QUrl &url, const QHttpServerRequest &request) {
        QString relative_path = url.toString();
        while(relative_path.endsWith('/')) {
            relative_path.chop(1);
        }



        QJsonObject res;
        QJsonArray sourceFolders = config["folders"].toArray();
        QJsonArray sourceFiles = config["files"].toArray();
        QJsonArray sourceVirtualRoots = config["virtual"].toArray();

        // if request path is not root location then check these rules
        bool canSee = true;
        bool canUpload = false;
        bool canDelete = false;
        bool canDownload = true;
        qint64 totalSize = 0;

        QString username = "";
        auto id = getAuthCookie(request);

        if (id){
            if (isAuthValid(*id)){
                username = getSessionUsername(*id);
            }
        }


        if (relative_path.isEmpty()){
            QJsonArray folders;
            QJsonArray files;
            for (const auto &folder : std::as_const(sourceFolders)) {
                auto _folder = folder.toObject();
                if (canAccessSource(username, _folder["canSee"])){
                    QFileInfo info(folder.toObject().value("src").toString());
                    QJsonObject _folder;
                    _folder["time"] = info.birthTime().toUTC().toString();
                    _folder["name"] = folder.toObject().value("name").toString();
                    folders.push_back(_folder);
                }
            }
            for (const auto &file : std::as_const(sourceFiles)) {
                auto _file = file.toObject();
                if (canAccessSource(username, _file["canSee"])) {
                    QFileInfo info(file.toObject().value("src").toString());
                    QJsonObject _file;
                    _file["time"] = info.birthTime().toUTC().toString();
                    _file["name"] = file.toObject().value("name").toString();
                    _file["size"] = info.size();
                    files.push_back(_file);
                }
            }

            for (const auto &virtualRoot : std::as_const(sourceVirtualRoots)) {
                auto _virtualRoot = virtualRoot.toObject();
                if (canAccessSource(username, _virtualRoot["canSee"])){
                    QJsonObject _folder;
                    _folder["name"] =  _virtualRoot.value("name").toString();
                    folders.push_back(_folder);
                }
            }


            res["folders"] = folders;
            res["files"] = files;
        }else{
            QStringList path = relative_path.split("/");

            bool isExist = false;
            for (const auto &folder : std::as_const(sourceFolders)){
                auto _folder = folder.toObject();
                if (path[0] == _folder.value("name").toString()){
                    path[0] = _folder.value("src").toString();
                    canSee = canAccessSource(username, _folder["canSee"]);
                    canUpload = canAccessSource(username, _folder["canUpload"]);
                    canDelete = canAccessSource(username, _folder["canDelete"]);
                    canDownload = canAccessSource(username, _folder["canDownload"]);
                    isExist = true;
                    break;
                }
            }

            if (isExist && canSee){
                QString absolute_path = path.join("/");
                if (path.length() == 1) absolute_path += "/"; // ex: D: exception
                auto folders = getDirectories(absolute_path);
                auto files = getFiles(absolute_path);
                totalSize = getDirectorySize(absolute_path);
                QJsonArray container;
                for (auto &folder : folders){
                    QFileInfo info(absolute_path+"/"+folder);
                    QJsonObject _folder;
                    _folder["time"] = info.birthTime().toUTC().toString();
                    _folder["name"] = folder;
                    container.append(_folder);
                }
                res["folders"] = container;

                container = QJsonArray();
                for (auto &file : files){
                    QFileInfo info(absolute_path+"/"+file);
                    QJsonObject _file;
                    _file["time"] = info.birthTime().toUTC().toString();
                    _file["name"] = file;
                    _file["size"] = info.size();
                    container.append(_file);
                }
                res["files"] = container;
            }

            /* find virtual root */

            if (path.length() == 1 && !isExist){
                for (const auto &virtualRoot : std::as_const(sourceVirtualRoots)) {
                    auto _virtualRoot = virtualRoot.toObject();
                    if (path[0] == _virtualRoot["name"]){
                        isExist = true;
                        QJsonArray virtualFolders = _virtualRoot.value("folders").toArray();
                        QJsonArray virtualFiles = _virtualRoot.value("files").toArray();
                        QJsonArray folders;
                        QJsonArray files;
                        for (const auto &folder : std::as_const(virtualFolders)) {
                            auto _folder = folder.toObject();
                            if (canAccessSource(username, _folder["canSee"])){
                                QFileInfo info(folder.toObject().value("src").toString());
                                QJsonObject _folder;
                                _folder["time"] = info.birthTime().toUTC().toString();
                                _folder["name"] = folder.toObject().value("name").toString();
                                folders.push_back(_folder);
                            }
                        }
                        for (const auto &file : std::as_const(virtualFiles)) {
                            auto _file = file.toObject();
                            if (canAccessSource(username, _file["canSee"])) {
                                QFileInfo info(file.toObject().value("src").toString());
                                QJsonObject _file;
                                _file["time"] = info.birthTime().toUTC().toString();
                                _file["name"] = file.toObject().value("name").toString();
                                _file["size"] = info.size();
                                files.push_back(_file);
                            }
                        }
                        res["folders"] = folders;
                        res["files"] = files;
                    }
                    if (isExist) break;
                }
            }else if (path.length() > 1 && !isExist){
                for (const auto &virtualRoot : std::as_const(sourceVirtualRoots)) {
                    auto _virtualRoot = virtualRoot.toObject();
                    if (path[0] == _virtualRoot["name"]){
                        QJsonArray virtualFolders = _virtualRoot.value("folders").toArray();
                        for (const auto &folder : std::as_const(virtualFolders)){
                            auto _folder = folder.toObject();
                            if (path[1] == _folder.value("name").toString()){
                                path[1] = _folder.value("src").toString();
                                canSee = canAccessSource(username, _folder["canSee"]);
                                canUpload = canAccessSource(username, _folder["canUpload"]);
                                canDelete = canAccessSource(username, _folder["canDelete"]);
                                canDownload = canAccessSource(username, _folder["canDownload"]);
                                isExist = true;
                                break;
                            }
                        }

                        path.removeFirst();

                        if (isExist && canSee){
                            QString absolute_path = path.join("/");

                            if (path.length() == 1) absolute_path += "/"; // ex: D: exception
                            auto folders = getDirectories(absolute_path);
                            auto files = getFiles(absolute_path);
                            totalSize = getDirectorySize(absolute_path);
                            QJsonArray container;
                            for (auto &folder : folders){
                                QFileInfo info(absolute_path+"/"+folder);
                                QJsonObject _folder;
                                _folder["time"] = info.birthTime().toUTC().toString();
                                _folder["name"] = folder;
                                container.append(_folder);
                            }
                            res["folders"] = container;

                            container = QJsonArray();
                            for (auto &file : files){
                                QFileInfo info(absolute_path+"/"+file);
                                QJsonObject _file;
                                _file["time"] = info.birthTime().toUTC().toString();
                                _file["name"] = file;
                                _file["size"] = info.size();
                                container.append(_file);
                            }
                            res["files"] = container;
                        }
                    }
                    if (isExist) break;
                }
            }
            /* find virtual root */
        }

        res["canSee"] = canSee;
        res["canUpload"] = canUpload;
        res["canDelete"] = canDelete;
        res["canDownload"] = canDownload;
        res["totalSize"] = totalSize;
        QJsonDocument doc(res);
        QHttpServerResponse response("application/json; charset=utf-8", doc.toJson(QJsonDocument::Compact), QHttpServerResponse::StatusCode::Ok);

        response.setHeaders(createHeaders(.cache = false));
        return response;
    });

    server.route("/__/api/deleteSources", QHttpServerRequest::Method::Options,
                 [](const QHttpServerRequest &) {
                     return sendStatus(QHttpServerResponse::StatusCode::Ok);
                 });

    server.route("/__/api/deleteSources", QHttpServerRequest::Method::Post, [](const QHttpServerRequest &request) {
        QJsonDocument doc;

        if (auto isInvalid = parseBody(doc, request.body())){
            return std::move(isInvalid.value());
        }

        bool canDelete = false;
        QString username = "";
        auto id = getAuthCookie(request);

        if (id){
            if (isAuthValid(*id)){
                username = getSessionUsername(*id);
            }
        }

        QJsonObject body = doc.object();
        QJsonArray folders = body.value("folders").toArray();
        QJsonArray files = body.value("files").toArray();
        QJsonArray sourceFolders = config["folders"].toArray();
        QJsonArray sourceVirtualRoots = config["virtual"].toArray();


        bool isExist = false;
        QString relative_path = QUrl::fromPercentEncoding(body.value("relative_path").toString().toUtf8());
        while(relative_path.endsWith('/')) {
            relative_path.chop(1);
        }
        QStringList path = relative_path.split("/");

        for (const auto &folder : std::as_const(sourceFolders)){
            auto _folder = folder.toObject();
            if (path[0] == _folder.value("name").toString()){
                path[0] = _folder.value("src").toString();
                canDelete = canAccessSource(username, _folder["canDelete"]);
                isExist = true;
                break;
            }
        }

        if (!isExist){
            for (const auto &virtualRoot : std::as_const(sourceVirtualRoots)) {
                auto _virtualRoot = virtualRoot.toObject();
                if (path[0] == _virtualRoot["name"]){
                    QJsonArray virtualFolders = _virtualRoot.value("folders").toArray();
                    for (const auto &folder : std::as_const(virtualFolders)){
                        auto _folder = folder.toObject();
                        if (path[1] == _folder.value("name").toString()){
                            path[1] = _folder.value("src").toString();
                            canDelete = canAccessSource(username, _folder["canDelete"]);
                            isExist = true;
                            path.removeFirst();
                            break;
                        }
                    }
                }
                if (isExist) break;
            }
        }

        if (!isExist)
            return sendStatus(QHttpServerResponse::StatusCode::BadRequest);

        if(!canDelete)
            return sendStatus(QHttpServerResponse::StatusCode::Unauthorized);

        QString absolute_path = path.join("/");
        for (const auto &folder : std::as_const(folders)){
            if (!removeDirectory(absolute_path+"/"+folder.toString())) return sendStatus(QHttpServerResponse::StatusCode::InternalServerError);
        }

        for (const auto &file : std::as_const(files)){
            if (!removeFile(absolute_path+"/"+file.toString())) return sendStatus(QHttpServerResponse::StatusCode::InternalServerError);
        }

        return sendStatus(QHttpServerResponse::StatusCode::Ok);
    });

    server.route("/__/api/makeDirectory", QHttpServerRequest::Method::Options,
                 [](const QHttpServerRequest &) {
                     return sendStatus(QHttpServerResponse::StatusCode::Ok);
                 });

    server.route("/__/api/makeDirectory", QHttpServerRequest::Method::Post, [](const QHttpServerRequest &request) {
        QJsonDocument doc;

        if (auto isInvalid = parseBody(doc, request.body())){
            return std::move(isInvalid.value());
        }

        QString username = "";
        auto id = getAuthCookie(request);

        if (id){
            if (isAuthValid(*id)){
                username = getSessionUsername(*id);
            }
        }

        bool canUpload = false;
        QJsonObject body = doc.object();
        QString name = body.value("name").toString();
        QString relative_path = QUrl::fromPercentEncoding(body.value("relative_path").toString().toUtf8());


        QJsonArray sourceFolders = config["folders"].toArray();
        QJsonArray sourceVirtualRoots = config["virtual"].toArray();

        bool isExist = false;
        while(relative_path.endsWith('/')) {
            relative_path.chop(1);
        }
        QStringList path = relative_path.split("/");

        for (const auto &folder : std::as_const(sourceFolders)){
            auto _folder = folder.toObject();
            if (path[0] == _folder.value("name").toString()){
                path[0] = _folder.value("src").toString();
                canUpload = canAccessSource(username, _folder["canUpload"]);
                isExist = true;
                break;
            }
        }

        if (!isExist && path.length() > 1){
            for (const auto &virtualRoot : std::as_const(sourceVirtualRoots)) {
                auto _virtualRoot = virtualRoot.toObject();
                if (path[0] == _virtualRoot["name"]){
                    QJsonArray virtualFolders = _virtualRoot.value("folders").toArray();
                    for (const auto &folder : std::as_const(virtualFolders)){
                        auto _folder = folder.toObject();
                        if (path[1] == _folder.value("name").toString()){\
                            path[1] = _folder.value("src").toString();
                            canUpload = canAccessSource(username, _folder["canUpload"]);
                            isExist = true;
                            path.removeFirst();
                            break;
                        }
                    }
                }
                if (isExist) break;
            }
        }

        if (!isExist)
            return sendStatus(QHttpServerResponse::StatusCode::BadRequest);

        if(!canUpload)
            return sendStatus(QHttpServerResponse::StatusCode::Unauthorized);

        QString absolute_path = path.join("/");
        if (makeDirectory(absolute_path+"/"+name))
            return sendStatus(QHttpServerResponse::StatusCode::Ok);
        else
            return sendStatus(QHttpServerResponse::StatusCode::BadRequest);
    });

    /* auth */
    server.route("/__/api/authenticate", [](const QHttpServerRequest &request) {
        auto id = getAuthCookie(request);

        if (!id)
            return sendStatus(QHttpServerResponse::StatusCode::Unauthorized);

        if (isAuthValid(*id)){
            return sendStatus(QHttpServerResponse::StatusCode::Ok);
        }else{
            QHttpHeaders headers = createHeaders();
            headers.append(QHttpHeaders::WellKnownHeader::SetCookie, clearAuth());
            QHttpServerResponse response(QHttpServerResponse::StatusCode::Unauthorized);
            response.setHeaders(headers);
            return response;
        }
    });

    server.route("/__/api/authorize", [](const QHttpServerRequest &request) {
        auto id = getAuthCookie(request);
        if (!id)
            return sendStatus(QHttpServerResponse::StatusCode::Unauthorized);

        if (isAuthValid(*id)){
            if (isAdmin(*id))
                return sendStatus(QHttpServerResponse::StatusCode::Ok);
            else{

            }
        }
        return sendStatus(QHttpServerResponse::StatusCode::Unauthorized);
    });

    server.route("/__/api/info", [](const QHttpServerRequest &request) {
        QString username = "";
        auto id = getAuthCookie(request);
        if (id){
            if (isAuthValid(*id)){
                username = getSessionUsername(*id);
            }
        }

        return sendStatus(username, QHttpServerResponse::StatusCode::Ok);
    });


    server.route("/__/api/logout", []() {
        QHttpHeaders headers = createHeaders();
        headers.append(QHttpHeaders::WellKnownHeader::SetCookie, clearAuth());
        QHttpServerResponse response(QHttpServerResponse::StatusCode::Unauthorized);
        response.setHeaders(headers);
        return response;
    });

    server.route("/__/api/login", QHttpServerRequest::Method::Options,
                 [](const QHttpServerRequest &) {
                     return sendStatus(QHttpServerResponse::StatusCode::Ok);
                 });

    server.route("/__/api/login", QHttpServerRequest::Method::Post, [](const QHttpServerRequest &request) {
        QJsonDocument doc;

        if (auto isInvalid = parseBody(doc, request.body())){
            return std::move(isInvalid.value());
        }

        QJsonObject body = doc.object();
        QString username = body.value("username").toString();
        QString password = hashing(body.value("password").toString());

        if (isAuthValid(username, password)){
            QHttpHeaders headers = createHeaders();
            headers.append(QHttpHeaders::WellKnownHeader::SetCookie, setAuth(generateSessionId(username)));
            QHttpServerResponse response(QHttpServerResponse::StatusCode::Ok);
            response.setHeaders(headers);
            return response;
        }else{
            return sendStatus(QHttpServerResponse::StatusCode::Unauthorized);
        }
    });

    /* Main page,  */

    server.route("/*", [](const QUrl &url, const QHttpServerRequest &request, QHttpServerResponder &responder) {
        QString relative_path = url.toString();

        while(relative_path.endsWith('/')) {
            relative_path.chop(1);
        }
        //logNormal(relative_path.toStdString());
        QStringList path = relative_path.split("/");


        bool isFound = false;
        bool _isFile = false;
        QJsonArray sourceFolders = config["folders"].toArray();
        QJsonArray sourceFiles = config["files"].toArray();
        QJsonArray sourceVirtualRoots = config["virtual"].toArray();


        QString username = "";
        auto id = getAuthCookie(request);

        if (id){
            if (isAuthValid(*id)){
                username = getSessionUsername(*id);
            }
        }
        bool canDownload = false;

        /* checking the path compare with the sources in config, if exist then break
         * isFound is just a flag to break out of the while loop if path is found in the sources
         * after checking if the path is not a file then continue check in the file system
         *
         * for example, if the request path is "Dir/file1.zip", first split the path into array
         * -> path = ["Dir", file1.zip"]
         * take the first element and compare with the sources alias
         * first compare with the source folders, if the name is found in the alias then replace it with the alias src
         * ex: if in source folders has an alias:
         * "folders": [
                {
                    "isHidden": false,
                    "name": "Dir",
                    "src": "D:/Dir/abc/xyz"
                }
            ],
            then replace the first element in the path array with the alias src
            -> path = ["D:/Dir/abc/xyz", file1.zip"]
            since the path is found in sources alias the while loop will break, but at this point we still not know
            if the path is a file or a directory, so one more checking isFile() to ensure the type.
            but before that we need to get the absolute path with path.join("/")
            -> absolute_path = "D:/Dir/abc/xyz/file1.zip"
            then give the absolute path to the isFile() function

            after all the checking, if the path is a file the serve the file to the client, if not serve the front page
            even if the path is valid or not, if not a valid path then the front page will show not thing (via the api /__/api/getSources).

            also this implement is for svelte, as i mentioned above, you can use inja for mvc style
            ex: after all the checking

            if(!_isFile){
                auto folders = getFolders(absolute_path);
                auto files = getFiles(absolute_path);
                if (!relative_path.isEmpty()) relative_path += "/";
                responder.write(filesPage(*folders, *files, relative_path).toUtf8(), "text/html", QHttpServerResponder::StatusCode::Ok);
                return;
            }else{
                same as below
            }
            see the httpTemplate.h for reference
         * */

        while (true){
            for (const auto &folder : std::as_const(sourceFolders)){
                auto _folder = folder.toObject();
                if (path[0] == _folder.value("name").toString()){
                    canDownload = canAccessSource(username, _folder["canDownload"]);
                    isFound = true;
                    path[0] = _folder.value("src").toString();
                    break;
                }
            }
            if (isFound) break;
            for (const auto &file : std::as_const(sourceFiles)){
                auto _file = file.toObject();
                if (path[0] == file.toObject().value("name").toString()){
                    canDownload = canAccessSource(username, _file["canDownload"]);
                    isFound = true;
                    _isFile = true;
                    path[0] = file.toObject().value("src").toString();
                    break;
                }
            }
            if (isFound) break;

            if (path.length() > 1){
                for (const auto virtualRoot :std::as_const(sourceVirtualRoots)) {
                    auto _virtualRoot = virtualRoot.toObject();
                    if (path[0] == _virtualRoot["name"]){
                        QJsonArray virtualFolders = _virtualRoot.value("folders").toArray();
                        QJsonArray virtualFiles = _virtualRoot.value("files").toArray();
                        path.removeFirst();
                        for (const auto &folder : std::as_const(virtualFolders)){
                            auto _folder = folder.toObject();
                            if (path[0] == _folder.value("name").toString()){
                                canDownload = canAccessSource(username, _folder["canDownload"]);
                                isFound = true;
                                path[0] = _folder.value("src").toString();
                                break;
                            }
                        }
                        if (isFound) break;
                        for (const auto &file : std::as_const(virtualFiles)){
                            auto _file = file.toObject();
                            if (path[0] == file.toObject().value("name").toString()){
                                canDownload = canAccessSource(username, _file["canDownload"]);
                                isFound = true;
                                _isFile = true;
                                path[0] = file.toObject().value("src").toString();
                                break;
                            }
                        }
                        break;
                    }
                }
            }
            break;
        }

        QString absolute_path = path.join("/");
        if (!_isFile && isFound) _isFile = isFile(absolute_path);

        QHttpHeaders headers = createHeaders(.cache = false);
        if(!_isFile){
            QString root = "./client/";
            QFile file(root+"index.html");
            file.open(QIODevice::ReadOnly);
            QByteArray fileContent = file.readAll();
            file.close();
            QMimeType mime = getMimeType(root+"index.html");

            headers.append("Content-Type", mime.name().toUtf8());
            responder.write(fileContent, headers, QHttpServerResponder::StatusCode::Ok);
        }else{
            // implement chunk serving, resumeable
            if (!canDownload) {
                responder.write("Forbidden content", "text/plain");
                responder.write(headers, QHttpServerResponder::StatusCode::Forbidden);
                return;
            }

            QFileInfo info(absolute_path);
            QFile *file = new QFile(path.join("/"));
            if (!file->open(QIODevice::ReadOnly)) {
                delete file;
                responder.write("Cannot open file", "text/plain");
                responder.write(headers, QHttpServerResponder::StatusCode::InternalServerError);
                //logNormal("can't open file");
                return;
            }

            qint64 fileSize = file->size();
            qint64 start = 0;
            qint64 end = fileSize - 1;

            auto rangeHeader = request.value("Range");
            if (!rangeHeader.isEmpty()) {
                static const QRegularExpression regex("bytes=(\\d+)-(\\d*)");
                QRegularExpressionMatch match = regex.match(rangeHeader);
                if (match.hasMatch()) {
                    start = match.captured(1).toLongLong();
                    if (!match.captured(2).isEmpty()) {
                        end = match.captured(2).toLongLong();
                    }
                }
            }

            if (start > fileSize - 1) {
                responder.write(QByteArray("File not found"), QByteArray("text/plain"), QHttpServerResponder::StatusCode::NotFound);
                file->close();
                delete file;
                responder.write(headers, QHttpServerResponder::StatusCode::InternalServerError);
                return;
            }

            end = qMin(end, fileSize - 1);
            //qint64 contentLength = end - start + 1;
            file->seek(start);
            //QByteArray data = file->read(contentLength);


            QMimeType mimeType = getMimeType(path.join("/"));

            auto status = (start > 0)   ? QHttpServerResponder::StatusCode::PartialContent
                                        : QHttpServerResponder::StatusCode::Ok;


            QHttpHeaders headers = createHeaders();
            headers.append("Content-Type", mimeType.name().toUtf8());
            //headers.append("Content-Type", "application/octet-stream");
            //headers.append("Content-Length", QByteArray::number(contentLength));
            headers.append("Content-Disposition", QString("inline; filename=\"%1\"").arg(info.fileName().toUtf8().toPercentEncoding()));
            headers.append("Accept-Ranges", "bytes");

            if (!rangeHeader.isEmpty()) {
                headers.append("Content-Range", QString("bytes %1-%2/%3")
                   .arg(start)
                   .arg(end)
                   .arg(fileSize)
                   .toUtf8());
            }


            responder.write(file, headers, status);
            // file->close();
            // delete file;
        }
    });



    server.addWebSocketUpgradeVerifier(&server, [](const QHttpServerRequest &request) {
        if (request.url().path() == "/__/ws") {
            QUrlQuery query(request.url());
            QString relative_path = query.queryItemValue("path");
            while(relative_path.endsWith('/')) {
                relative_path.chop(1);
            }

            QString username = "";
            auto id = getAuthCookie(request);

            if (id){
                if (isAuthValid(*id)){
                    username = getSessionUsername(*id);
                }
            }

            QStringList path = relative_path.split("/");
            bool isExist = false;
            bool canUpload = false;
            QJsonArray sourceFolders = config["folders"].toArray();
            QJsonArray sourceVirtualRoots = config["virtual"].toArray();
            for (const auto &folder : std::as_const(sourceFolders)){
                auto _folder = folder.toObject();
                if (path[0] == _folder.value("name").toString()){
                    canUpload = canAccessSource(username, _folder["canUpload"]);
                    isExist = true;
                    break;
                }
            }

            if (!isExist){
                for (const auto &virtualRoot : std::as_const(sourceVirtualRoots)) {
                    auto _virtualRoot = virtualRoot.toObject();
                    if (path[0] == _virtualRoot["name"]){
                        QJsonArray virtualFolders = _virtualRoot.value("folders").toArray();
                        for (const auto &folder : std::as_const(virtualFolders)){
                            auto _folder = folder.toObject();
                            if (path[1] == _folder.value("name").toString()){
                                canUpload = canAccessSource(username, _folder["canUpload"]);
                                isExist = true;
                                break;
                            }
                        }
                    }
                    if (isExist) break;
                }
            }

            if (!isExist)
                return QHttpServerWebSocketUpgradeResponse::passToNext();

            if(!canUpload)
                return QHttpServerWebSocketUpgradeResponse::passToNext();

            return QHttpServerWebSocketUpgradeResponse::accept();
        }
        return QHttpServerWebSocketUpgradeResponse::passToNext();
    });

    // Handle new WebSocket connections (note: no args)
    QObject::connect(&server, &QAbstractHttpServer::newWebSocketConnection, [&server]() {
        // Grab the pending connection
        std::unique_ptr<QWebSocket> wsPtr = server.nextPendingWebSocketConnection();
        if (!wsPtr) return;

        // Transfer ownership to Qt's parent/cleanup model
        QWebSocket *ws = wsPtr.release();         // release from unique_ptr
        ws->setParent(&server);                   // or manage in your own container

        // qDebug() << "WebSocket connected";

        QObject::connect(ws, &QWebSocket::textMessageReceived, [ws](const QString &message) {
            QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
            if (!doc.isObject())
                return;
            QJsonObject obj = doc.object();
            uint8_t event = obj["event"].toInt();
            QJsonValue data = obj["data"];
            // qDebug() << event << data;
            onEvent(ws, event, data);
        });

        QObject::connect(ws, &QWebSocket::binaryMessageReceived, [ws](const QByteArray &data) {
            auto *ctx = uploads.value(ws, nullptr);
            if (!ctx || !ctx->file.isOpen()) return;
            ctx->file.write(data);
            ctx->receivedSize += data.size();
        });

        QObject::connect(ws, &QWebSocket::disconnected, [ws]() {
            // qDebug() << "WebSocket disconnected";
            removeUpload(ws);
            ws->deleteLater();
        });
    });



    if (!tcpStart(tcpServer, server, port)) return -1;
    // quint16 port = tcpServer->serverPort();
    //tcpServer.release();
    checkSessions();
    logNormal("Main page: http://localhost:"+QString::number(port).toStdString()+"/");
    logNormal("Admin page: http://localhost:"+QString::number(port).toStdString()+"/__/admin/");
    return a.exec();
}
