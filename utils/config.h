#ifndef CONFIG_H
#define CONFIG_H

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRandomGenerator64>
#include "utils/stringUtils.h"
#include "utils/qtbcrypt.h"

extern QJsonObject config;

bool writeConfig();

bool getConfig();



#endif // CONFIG_H
