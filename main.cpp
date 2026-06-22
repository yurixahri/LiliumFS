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

#include "utils/tar.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (!getConfig()) return 1;

    const uint16_t port = config.port;

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
        return getDrives();
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

        return addVirtualDirectory(request);
    });

    server.route("/__/admin/api/changeVirtualRoot", QHttpServerRequest::Method::Options,
                 [](const QHttpServerRequest &) {
                     return sendStatus(QHttpServerResponse::StatusCode::Ok);
                 });

    server.route("/__/admin/api/changeVirtualRoot", QHttpServerRequest::Method::Post, [](const QHttpServerRequest &request) {
        if (!request.remoteAddress().isLoopback() && !isAuthorized(request)) {
            return sendStatus("Forbidden", QHttpServerResponse::StatusCode::Forbidden);
        }

        return changeVirtualDirectory(request);
    });

    server.route("/__/admin/api/deleteVirtualRoot", QHttpServerRequest::Method::Options,
                 [](const QHttpServerRequest &) {
                     return sendStatus(QHttpServerResponse::StatusCode::Ok);
                 });

    server.route("/__/admin/api/deleteVirtualRoot", QHttpServerRequest::Method::Post, [](const QHttpServerRequest &request) {
        if (!request.remoteAddress().isLoopback() && !isAuthorized(request)) {
            return sendStatus("Forbidden", QHttpServerResponse::StatusCode::Forbidden);
        }

        return deleteVirtualDirectory(request);
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
        settings["port"] = config.port;

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
        config.port = body["port"].toInt();
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

        if (config.accounts.size() == 0){
            return sendStatus(QHttpServerResponse::StatusCode::Ok);
        }else{
            return sendStatus(QHttpServerResponse::StatusCode::ServiceUnavailable);
        }
    });

    server.route("/__/admin/*", [](const QUrl &url, const QHttpServerRequest &request) {
        // if (!request.remoteAddress().isLoopback() && !isAuthorized(request)) {
        //     return sendStatus("Forbidden", QHttpServerResponse::StatusCode::Forbidden);
        // }

        QString path = url.path();
        while(path.endsWith('/')) {
            path.chop(1);
        }

        //logNormal(path.toStdString());
        QString root = QCoreApplication::applicationDirPath() + "/client/__/admin/";
        if (!isPathExist(root+path)) {
            return sendStatus("This path is not exist", QHttpServerResponder::StatusCode::BadRequest);
        }

        if (!isFile(root+path)){
            if (!path.isEmpty()) path += "/";
            if(isPathExist(root+path+"index.html")){
                QFile file(root+path+"index.html");
                if (!file.open(QIODevice::ReadOnly)) {
                    return sendStatus(QHttpServerResponder::StatusCode::NotFound);
                }

                QMimeType mime = getMimeType(root+path+"index.html");
                QHttpServerResponse response(file.readAll());
                QHttpHeaders headers = createHeaders(.cache = false);
                headers.append("Content-Type", mime.name().toUtf8());
                response.setHeaders(headers);
                return response;
            }else{
                return sendStatus("No page here", QHttpServerResponder::StatusCode::BadRequest);
            }
        }else{
            QFile file(root+path);
            if (!file.open(QIODevice::ReadOnly)) {
                return sendStatus(QHttpServerResponder::StatusCode::NotFound);
            }

            QMimeType mime = getMimeType(root+path);
            QHttpServerResponse response(file.readAll());
            QHttpHeaders headers = createHeaders(.cache = false);
            headers.append("Content-Type", mime.name().toUtf8());
            response.setHeaders(headers);
            return response;
        }
    });

    // For svelte static files
    server.route("/_app/*", [](const QUrl &url) {
        QString path = url.path();
        while(path.endsWith('/')) {
            path.chop(1);
        }

        QString root = QCoreApplication::applicationDirPath() + "/client/_app/";
        if (!isPathExist(root+path)) {
            return sendStatus("This path is not exist", QHttpServerResponder::StatusCode::BadRequest);
        }

        if (isFile(root+path)){
            QFile file(root+path);
            if (!file.open(QIODevice::ReadOnly)) {
                return sendStatus(QHttpServerResponder::StatusCode::NotFound);
            }

            QMimeType mime = getMimeType(root+path);
            QHttpServerResponse response(file.readAll());
            QHttpHeaders headers = createHeaders(.cache = false);
            headers.append("Content-Type", mime.name().toUtf8());
            response.setHeaders(headers);
            return response;
        }else{
            return sendStatus(QHttpServerResponder::StatusCode::NotFound);
        }
    });

    server.route("/_static/*", [](const QUrl &url) {
        QString path = url.path();
        while(path.endsWith('/')) {
            path.chop(1);
        }

        QString root = QCoreApplication::applicationDirPath() + "/client/";
        if (!isPathExist(root+path)) {
            return sendStatus("This path is not exist", QHttpServerResponder::StatusCode::BadRequest);
        }

        if (isFile(root+path)){
            QFile file(root+path);
            if (!file.open(QIODevice::ReadOnly)) {
                return sendStatus(QHttpServerResponder::StatusCode::NotFound);
            }

            QMimeType mime = getMimeType(root+path);
            QHttpServerResponse response= QHttpServerResponse::fromFile(root+path);
            QHttpHeaders headers = createHeaders(.cache = false);
            headers.append("Content-Type", mime.name().toUtf8());
            headers.append("Cache-Control", "no-store, must-revalidate");
            response.setHeaders(headers);
            return response;
        }else{
            return sendStatus(QHttpServerResponder::StatusCode::NotFound);
        }
    });

    /* user api */
    server.route("/__/api/getSources/*", [](const QUrl &url, const QHttpServerRequest &request) {
        QReadLocker locker(&config.lock);

        QString relative_path = url.path();
        while(relative_path.endsWith('/')) {
            relative_path.chop(1);
        }

        // if request path is not root location then check these rules
        bool can_see = true;
        bool can_upload = false;
        bool can_delete = false;
        bool can_download = false;
        qint64 total_size = 0;

        QString username = "";
        auto id = getAuthCookie(request);

        if (id){
            if (isAuthValid(*id)){
                username = getSessionUsername(*id);
            }
        }

        QList<directory_t> res_dirs;
        QList<file_t> res_files;
        if (relative_path.isEmpty()){
            // QJsonArray dirs;
            // QJsonArray files;
            for (auto &d : config.dirs) {
                if (canAccessSource(username, d.can_see)){
                    QFileInfo info(d.src);
                    d.time = info.birthTime().toUTC().toString();
                    res_dirs.push_back(d);
                }
            }
            for (auto &file : config.files) {
                if (canAccessSource(username, file.can_see)) {
                    QFileInfo info(file.src);
                    file.time = info.birthTime().toUTC().toString();
                    file.size = info.size();
                    res_files.push_back(file);
                }
            }

            for (auto &vd: config.vds) {
                if (canAccessSource(username, vd.can_see)){
                    directory_t dir;
                    dir.name =  vd.name;
                    res_dirs.push_back(dir);
                }
            }

        }else{
            QStringList path = relative_path.split("/");

            bool is_exist = false;
            for (auto &dir : config.dirs){
                if (path[0] == dir.name){
                    path[0] = dir.src;
                    can_see = canAccessSource(username, dir.can_see);
                    can_download = canAccessSource(username, dir.can_download);
                    can_upload = canAccessSource(username, dir.can_upload);
                    can_delete = canAccessSource(username, dir.can_delete);
                    is_exist = true;
                    break;
                }
            }

            if (is_exist && can_see){
                QString absolute_path = path.join("/");
                if (path.length() == 1) absolute_path += "/"; // ex: D: exception
                auto n_dirs = getDirectories(absolute_path);
                auto n_files = getFiles(absolute_path);
                total_size = getDirectorySize(absolute_path);

                for (auto &n_dir : n_dirs){
                    directory_t dir;
                    QFileInfo info(absolute_path+"/"+n_dir);
                    dir.name = n_dir;
                    dir.time = info.birthTime().toUTC().toString();
                    res_dirs.push_back(dir);
                }

                for (auto &n_file : n_files){
                    file_t file;
                    QFileInfo info(absolute_path+"/"+n_file);
                    file.name = n_file;
                    file.time = info.birthTime().toUTC().toString();
                    file.size = info.size();
                    res_files.push_back(file);
                }
            }

            /* find virtual root */

            if (path.length() == 1 && !is_exist){
                for (auto &vd : config.vds) {
                    if (path[0] == vd.name){
                        is_exist = true;
                        for (auto &dir: vd.dirs) {
                            if (canAccessSource(username, dir.can_see)){
                                QFileInfo info(dir.src);
                                dir.time = info.birthTime().toUTC().toString();
                                res_dirs.push_back(dir);
                            }
                        }
                        for (auto &file : vd.files) {
                            if (canAccessSource(username, file.can_see)) {
                                QFileInfo info(file.src);
                                file.time = info.birthTime().toUTC().toString();
                                file.size = info.size();
                                res_files.push_back(file);
                            }
                        }
                    }
                    if (is_exist) break;
                }
            }else if (path.length() > 1 && !is_exist){
                for (auto &vd : config.vds) {
                    if (path[0] == vd.name){
                        for (auto &dir : vd.dirs){
                            if (path[1] == dir.name){
                                path[1] = dir.src;
                                can_see = canAccessSource(username, dir.can_see);
                                can_download = canAccessSource(username, dir.can_download);
                                can_upload = canAccessSource(username, dir.can_upload);
                                can_delete = canAccessSource(username, dir.can_delete);
                                is_exist = true;
                                break;
                            }
                        }

                        path.removeFirst();

                        if (is_exist && can_see){
                            QString absolute_path = path.join("/");

                            if (path.length() == 1) absolute_path += "/"; // ex: D: exception
                            auto n_dirs = getDirectories(absolute_path);
                            auto n_files = getFiles(absolute_path);
                            total_size = getDirectorySize(absolute_path);

                            for (auto &n_dir : n_dirs){
                                directory_t dir;
                                QFileInfo info(absolute_path+"/"+n_dir);
                                dir.name = n_dir;
                                dir.time = info.birthTime().toUTC().toString();
                                res_dirs.push_back(dir);
                            }

                            for (auto &n_file : n_files){
                                file_t file;
                                QFileInfo info(absolute_path+"/"+n_file);
                                file.name = n_file;
                                file.time = info.birthTime().toUTC().toString();
                                file.size = info.size();
                                res_files.push_back(file);
                            }
                        }
                    }
                    if (is_exist) break;
                }
            }
            /* find virtual root */
        }

        QVariantMap res;

        QVariantList dirs;
        for(const auto &d : res_dirs) dirs << dir_to_map_client(d);
        res["dirs"] = dirs;

        QVariantList files;
        for(const auto &f : res_files) files << file_to_map_client(f);
        res["files"] = files;

        res["can_see"] = can_see;
        res["can_upload"] = can_upload;
        res["can_delete"] = can_delete;
        res["can_download"] = can_download;
        res["total_size"] = total_size;

        QJsonDocument doc = QJsonDocument::fromVariant(res);
        QHttpServerResponse response("application/json; charset=utf-8", doc.toJson(QJsonDocument::Compact), QHttpServerResponse::StatusCode::Ok);

        response.setHeaders(createHeaders(.cache = false));
        return response;
    });

    server.route("/__/api/deleteSources", QHttpServerRequest::Method::Options,
                 [](const QHttpServerRequest &) {
                     return sendStatus(QHttpServerResponse::StatusCode::Ok);
                 });

    server.route("/__/api/deleteSources", QHttpServerRequest::Method::Post, [](const QHttpServerRequest &request) {
        QReadLocker locker(&config.lock);

        QJsonDocument doc;

        if (auto isInvalid = parseBody(doc, request.body())){
            return std::move(isInvalid.value());
        }

        bool can_delete = false;
        QString username = "";
        auto id = getAuthCookie(request);

        if (id){
            if (isAuthValid(*id)){
                username = getSessionUsername(*id);
            }
        }

        QJsonObject body = doc.object();
        QJsonArray dirs = body.value("dirs").toArray();
        QJsonArray files = body.value("files").toArray();

        bool is_exist = false;
        QString relative_path = QUrl::fromPercentEncoding(body.value("relative_path").toString().toUtf8());
        while(relative_path.endsWith('/')) {
            relative_path.chop(1);
        }
        QStringList path = relative_path.split("/");

        for (auto &dir: config.dirs){
            if (path[0] == dir.name){
                path[0] = dir.src;
                can_delete = canAccessSource(username, dir.can_delete);
                is_exist = true;
                break;
            }
        }

        if (!is_exist){
            for (auto &vd : config.vds) {
                if (path[0] == vd.name){
                    for (auto &dir : config.dirs){
                        if (path[1] == dir.name){
                            path[1] = dir.src;
                            can_delete = canAccessSource(username, dir.can_delete);
                            is_exist = true;
                            path.removeFirst();
                            break;
                        }
                    }
                }
                if (is_exist) break;
            }
        }

        if (!is_exist)
            return sendStatus(QHttpServerResponse::StatusCode::BadRequest);

        if(!can_delete)
            return sendStatus(QHttpServerResponse::StatusCode::Unauthorized);

        QString absolute_path = path.join("/");
        for (const auto &dir : std::as_const(dirs)){
            if (!removeDirectory(absolute_path+"/"+dir.toString())) return sendStatus(QHttpServerResponse::StatusCode::InternalServerError);
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
        QReadLocker locker(&config.lock);

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

        bool can_upload = false;
        QJsonObject body = doc.object();
        QString name = body.value("name").toString();
        QString relative_path = QUrl::fromPercentEncoding(body.value("relative_path").toString().toUtf8());

        bool is_exist = false;
        while(relative_path.endsWith('/')) {
            relative_path.chop(1);
        }
        QStringList path = relative_path.split("/");

        for (auto &dir : config.dirs){
            if (path[0] == dir.name){
                path[0] = dir.src;
                can_upload = canAccessSource(username, dir.can_upload);
                is_exist = true;
                break;
            }
        }

        if (!is_exist && path.length() > 1){
            for (auto &vd : config.vds) {
                if (path[0] == vd.name){
                    for (auto &dir : vd.dirs){
                        if (path[1] == dir.name){
                            path[1] = dir.src;
                            can_upload = canAccessSource(username, dir.can_upload);
                            is_exist = true;
                            path.removeFirst();
                            break;
                        }
                    }
                }
                if (is_exist) break;
            }
        }

        if (!is_exist)
            return sendStatus(QHttpServerResponse::StatusCode::BadRequest);

        if(!can_upload)
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

    server.route("/__/api/zip", QHttpServerRequest::Method::Options,
                 [](const QHttpServerRequest &) {
                     return sendStatus(QHttpServerResponse::StatusCode::Ok);
                 });

    server.route("/__/api/zip", QHttpServerRequest::Method::Post, [](const QHttpServerRequest &request, QHttpServerResponder &responder) {
        QReadLocker locker(&config.lock);

        QUrlQuery query(request.body());
        QByteArray data = query.queryItemValue("data", QUrl::FullyDecoded).toUtf8();
        QJsonDocument doc;

        if (auto isInvalid = parseBody(doc, data)){
            return;
        }

        QJsonObject body = doc.object();
        QString relative_path = body.value("relative_path").toString();
        QJsonArray dirs = body.value("dirs").toArray();
        QJsonArray files = body.value("files").toArray();

        while(relative_path.endsWith('/')) {
            relative_path.chop(1);
        }
        QStringList path = relative_path.split("/");
        QString file_name = path.last();
        QString username = "";
        auto id = getAuthCookie(request);

        if (id){
            if (isAuthValid(*id)){
                username = getSessionUsername(*id);
            }
        }

        bool can_download = false;
        while (true){
            for (auto &dir : config.dirs){
                if (path[0] == dir.name){
                    can_download = canAccessSource(username, dir.can_download);
                    path[0] = dir.src;
                    break;
                }
            }
            if (can_download) break;

            if (path.length() > 1){
                for (auto &vd : config.vds) {
                    if (path[0] == vd.name){
                        path.removeFirst();
                        for (auto &dir : vd.dirs){
                            if (path[0] == dir.name){
                                can_download = canAccessSource(username, dir.can_download);
                                path[0] = dir.src;
                                break;
                            }
                        }
                        if (can_download) break;
                    }
                }
            }
            break;
        }

        if (can_download){
            auto shared_responder = std::make_shared<QHttpServerResponder>(std::move(responder));
            QString absolute_path = path.join("/");
            QThreadPool::globalInstance()->start([shared_responder, dirs, files, absolute_path, file_name]() {
                QHttpHeaders headers;
                headers.append("Content-Type", "application/x-gzip");
                headers.append("Content-Encoding", "identity");
                headers.append("Content-Disposition", "attachment; filename=\""+file_name+".tar\"");

                // Headers must be written on the main thread too
                QMetaObject::invokeMethod(qApp, [shared_responder, headers]() {
                    shared_responder->writeBeginChunked(headers, QHttpServerResponder::StatusCode::Ok);
                }, Qt::BlockingQueuedConnection);

                archive* a = archive_write_new();
                // archive_write_add_filter_gzip(a);
                archive_write_set_format_pax_restricted(a);
                archive_write_set_options(a, "no-null");
                archive_write_set_bytes_in_last_block(a, 1);
                archive_write_open(a, (void*)&shared_responder, nullptr, archiveWriteCallback, nullptr);

                for (auto const &dir : dirs){
                    if(addArchiveEntry(a, dir.toString(), absolute_path+"/"+dir.toString()) == ARCHIVE_FATAL) break;
                }
                for (auto const &file : files){
                    if(addArchiveEntry(a, file.toString(), absolute_path+"/"+file.toString()) == ARCHIVE_FATAL) break;
                }

                archive_write_close(a);
                archive_write_free(a);

                // Finalize on main thread
                QMetaObject::invokeMethod(qApp, [shared_responder]() {
                    if (!shared_responder->isResponseCanceled()) {
                        shared_responder->writeEndChunked(" ");
                    }
                }, Qt::BlockingQueuedConnection);
            });
        }
    });

    /* Main page,  */

    server.route("/*", [](const QUrl &url, const QHttpServerRequest &request, QHttpServerResponder &responder) {
        QReadLocker locker(&config.lock);

        QString relative_path = url.path();

        while(relative_path.endsWith('/')) {
            relative_path.chop(1);
        }
        //logNormal(relative_path.toStdString());
        QStringList path = relative_path.split("/");


        QString username = "";
        auto id = getAuthCookie(request);

        if (id){
            if (isAuthValid(*id)){
                username = getSessionUsername(*id);
            }
        }
        bool is_file = false;
        bool can_download = false;

        /* checking the path compare with the sources in config, if exist then break
         * is_found is just a flag to break out of the while loop if path is found in the sources
         * after checking if the path is not a file then continue check in the file system
         *
         * for example, if the request path is "Dir/file1.zip", first split the path into array
         * -> path = ["Dir", file1.zip"]
         * take the first element and compare with the sources alias
         * first compare with the source dirs, if the name is found in the alias then replace it with the alias src
         * ex: if in source dirs has an alias:
         * "dirs": [
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

            if(!is_file){
                auto dirs = getdirs(absolute_path);
                auto files = getFiles(absolute_path);
                if (!relative_path.isEmpty()) relative_path += "/";
                responder.write(filesPage(*dirs, *files, relative_path).toUtf8(), "text/html", QHttpServerResponder::StatusCode::Ok);
                return;
            }else{
                same as below
            }
            see the htmlTemplate.h for reference
         * */

        while (true){
            for (auto &dir : config.dirs){
                if (path[0] == dir.name){
                    can_download = canAccessSource(username, dir.can_download);
                    path[0] = dir.src;
                    break;
                }
            }
            if (can_download) break;
            for (auto &file : config.files){
                if (path[0] == file.name){
                    can_download = canAccessSource(username, file.can_download);
                    is_file = true;
                    path[0] = file.src;
                    break;
                }
            }
            if (can_download) break;

            if (path.length() > 1){
                for (auto &vd : config.vds) {
                    if (path[0] == vd.name){
                        path.removeFirst();
                        for (auto &dir : vd.dirs){
                            if (path[0] == dir.name){
                                can_download = canAccessSource(username, dir.can_download);
                                path[0] = dir.src;
                                break;
                            }
                        }
                        if (can_download) break;
                        for (auto &file : vd.files){
                            if (path[0] == file.name){
                                can_download = canAccessSource(username, file.can_download);
                                is_file = true;
                                path[0] = file.src;
                                break;
                            }
                        }
                        if (can_download) break;
                    }
                }
            }
            break;
        }

        QString absolute_path = path.join("/");
        if (!is_file && can_download) is_file = isFile(absolute_path);

        QHttpHeaders headers = createHeaders(.cache = false);
        if(!is_file){
            QString root = QCoreApplication::applicationDirPath() + "/client/";
            QFile file(root+"index.html");
            if (!file.open(QIODevice::ReadOnly)) {
                return ;
            }
            QMimeType mime = getMimeType(root+"index.html");
            headers.append("Content-Type", mime.name().toUtf8());
            responder.write(file.readAll(), headers, QHttpServerResponder::StatusCode::Ok);
        }else{
            // implement chunk serving, resumeable
            if (!can_download) {
                responder.write("Forbidden content", "text/plain");
                responder.write(headers, QHttpServerResponder::StatusCode::Forbidden);
                return;
            }

            QFileInfo info(absolute_path);
            QFile *file = new QFile(absolute_path);
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
            bool is_exist = false;
            bool can_upload = false;
            for (auto &dir : config.dirs){
                if (path[0] == dir.name){
                    can_upload = canAccessSource(username, dir.can_upload);
                    is_exist = true;
                    break;
                }
            }

            if (!is_exist){
                for (auto &vd : config.vds) {
                    if (path[0] == vd.name){
                        for (auto &dir : vd.dirs){
                            if (path[1] == dir.name){
                                can_upload = canAccessSource(username, dir.can_upload);
                                is_exist = true;
                                break;
                            }
                        }
                    }
                    if (is_exist) break;
                }
            }

            if (!is_exist)
                return QHttpServerWebSocketUpgradeResponse::passToNext();

            if(!can_upload)
                return QHttpServerWebSocketUpgradeResponse::passToNext();

            return QHttpServerWebSocketUpgradeResponse::accept();
        }
        return QHttpServerWebSocketUpgradeResponse::passToNext();
    });

    // Handle new WebSocket connections (note: no args)
    QObject::connect(&server, &QAbstractHttpServer::newWebSocketConnection, [&server]() {
        // Grab the pending connection
        std::unique_ptr<QWebSocket> ws_ptr = server.nextPendingWebSocketConnection();
        if (!ws_ptr) return;

        // Transfer ownership to Qt's parent/cleanup model
        QWebSocket *ws = ws_ptr.release();         // release from unique_ptr
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
    QString version = "0.1.2";
    logNormal("LiliumFS version "+version.toStdString());
    logNormal("Main page: http://localhost:"+QString::number(port).toStdString()+"/");
    logNormal("Admin page: http://localhost:"+QString::number(port).toStdString()+"/__/admin/");
    return a.exec();
}
