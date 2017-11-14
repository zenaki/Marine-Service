#include "config.h"

config::config()
{
}

QStringList config::read(QString obj)
{
    QStringList result;
    if (loadConfig(config::Json)) {
        JsonDoc = QJsonDocument::fromJson(data_json);
        QJsonObject object = JsonDoc.object();
        QJsonValue value = object.value(obj);
        if (obj == "CONFIG") {
            if (object.value(obj).isArray()) {
                QJsonArray array = value.toArray();
                foreach (const QJsonValue & v, array) {
                    result.append(v.toObject().value("PLUGIN").toString());
                    result.append(QString::number(v.toObject().value("INTERVAL").toInt()));
                }
            } else {
                result.append(value.toObject().value("PLUGIN").toString());
                result.append(QString::number(value.toObject().value("INTERVAL").toInt()));
            }
        }
    }
    return result;
}

void config::write(QJsonObject &json) const //Default
{
    QJsonArray configArray;
    QJsonObject configObject;
    configObject["PLUGIN"] = "/usr/local/bin/plugins/SkyWave";
    configObject["INTERVAL"] = 300000;    //milis
    configArray.append(configObject);
    json["CONFIG"] = configArray;
}

bool config::loadConfig(config::SaveFormat saveFormat)
{
    QString path;
    if (saveFormat == Json) {
        path = PATH_CONFIGURATION_JSON;
    } else {
        path = PATH_CONFIGURATION_BIN;
    }
    QFile loadFile(path);
    if (!loadFile.exists()) {
        QDir dir;
        dir.mkpath(PATH_MONITA);
    }
    if (loadFile.open(QIODevice::ReadWrite)) {
        QByteArray readFile = loadFile.readAll();
        if (readFile.isEmpty()) {
            QJsonObject configObject;
            write(configObject);
            QJsonDocument saveDoc(configObject);
            loadFile.write(saveFormat == Json
                ? saveDoc.toJson()
                : saveDoc.toBinaryData());
        }
        data_json = readFile;
        return true;
    } else {
        return false;
    }

//    read(loadDoc.object());
}
//! [3]

//! [4]
bool config::saveConfig(config::SaveFormat saveFormat) const
{
    QString path;
    if (saveFormat == Json) {
        path = PATH_CONFIGURATION_JSON;
    } else {
        path = PATH_CONFIGURATION_BIN;
    }
    QFile saveFile(path);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Couldn't save config ..";
        return false;
    }

    QJsonObject configObject;
    write(configObject);
    QJsonDocument saveDoc(configObject);
    saveFile.write(saveFormat == Json
        ? saveDoc.toJson()
        : saveDoc.toBinaryData());

    return true;
}
