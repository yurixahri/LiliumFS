#include "utils/config.h"

config_t config;


// bool writeConfig(){
//     QJsonDocument doc(config);
//     QFile file("./config.json");
//     if (!file.open(QIODevice::WriteOnly)) {
//         qWarning() << "Couldn't open file for writing:" << file.errorString();
//         return false;
//     }

//     file.write(doc.toJson());  // Write as JSON text
//     file.close();
//     return true;
// }

bool writeConfig(){
    QVariantMap root;
    root["port"] = config.port;
    root["encryption_key"] = config.encryption_key;

    QVariantList accounts;
    for(const auto &account : config.accounts) accounts << account_to_map(account);
    root["accounts"] = accounts;

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
    QFile file("./config.json");
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(doc.toJson());
    return true;
}

bool getConfig(){
    QReadLocker locker(&config.lock);

    if (!QFile::exists("./config.json")) {
        logWarning("Couldn't find config file, exported default config");
        config.port = 9090;
        config.encryption_key = QtBCrypt::generateSalt();
        writeConfig();
        return true;
    }

    QFile file("./config.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        logError("Couldn't open file:" + file.errorString().toStdString());
        return false;
    }

    QVariantMap data = QJsonDocument::fromJson(file.readAll()).toVariant().toMap();

    file.close();

    config.port = data.value("port", 9090).toUInt();
    config.encryption_key = data.value("encryption_key").toString();


    if (config.encryption_key.isEmpty()) {
        config.encryption_key = QtBCrypt::generateSalt();
    }

    QVariantList dirs = data["dirs"].toList();
    QVariantList files = data["files"].toList();
    QVariantList vds = data["vds"].toList();
    QVariantList accounts = data["accounts"].toList();

    for (const QVariant &v : dirs) {
        QVariantMap m = v.toMap();
        config.dirs << map_to_dir(m);
    }

    for (const QVariant &v : files) {
        QVariantMap m = v.toMap();
        config.files << map_to_file(m);
    }

    for (const QVariant &v : vds) {
        QVariantMap m = v.toMap();
        config.vds << map_to_vd(m);
    }

    for (const QVariant &v : accounts) {
        QVariantMap m = v.toMap();
        config.accounts << map_to_account(m);
    }

    writeConfig();
    return true;
}


