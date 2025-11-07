#include "utils/config.h"

QJsonObject config;



bool writeConfig(){
    QJsonDocument doc(config);
    QFile file("./config.json");
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Couldn't open file for writing:" << file.errorString();
        return false;
    }

    file.write(doc.toJson());  // Write as JSON text
    file.close();
    return true;
}

bool getConfig(){

    if (!QFile::exists("./config.json")) {
        logWarning("Couldn't find config file, exported default config");
        config["port"] = 9090;
        config["folders"] = QJsonArray();
        config["files"] = QJsonArray();
        config["accounts"] = QJsonArray();
        config["virtual"] = QJsonArray();
        config["encryption_key"] = QtBCrypt::generateSalt();
        writeConfig();
        return true;
    }

    QFile file("./config.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        logError("Couldn't open file:" + file.errorString().toStdString());
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        logError("JSON parse error:" + parseError.errorString().toStdString());
        logWarning("Generate default config");
        config["port"] = 9090;
        config["folders"] = QJsonArray();
        config["files"] = QJsonArray();
        config["accounts"] = QJsonArray();
        config["encryption_key"] = QtBCrypt::generateSalt();
        writeConfig();
        return true;
    }

    if (!doc.isObject()) {
        logError("Config file does not contain a JSON object!");
        logWarning("Generate default config");
        config["port"] = 9090;
        config["folders"] = QJsonArray();
        config["files"] = QJsonArray();
        config["accounts"] = QJsonArray();
        config["encryption_key"] = QtBCrypt::generateSalt();
        writeConfig();
        return true;
    }

    config = doc.object();
    if (config["port"].isNull() || config["port"].isUndefined() || !config["port"].isDouble()){
        logWarning("Found config but missing or an invalid port, default port will be used");
        config["port"] = 9090;
        writeConfig();
    }

    if (config["encryption_key"].isNull() || config["encryption_key"].isUndefined() || !config["encryption_key"].isString() || config["encryption_key"].toString().isEmpty()){
        logWarning("Found config but missing or an invalid encryption key, generating new one");
        config["encryption_key"] = QtBCrypt::generateSalt();
        writeConfig();
    }
    return true;
}


