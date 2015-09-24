/*
 *
 *
 * Author: Samuel Isuani <sisuani@gmail.com>
 *
 *
 */

#include "serverpackagemanager.h"

#include <QVariantMap>
#include <QDebug>
#include <QJson/Serializer>
#include <QJson/Parser>

#include "package.h"

#define CMD_OK          "cmd_ok"
#define RADIO_NOT_VALID "radio_not_valid"
#define CMD_ERROR       "cmd_error"
#define PKG_CMD_ERROR   "{ \"status\": \"cmd_error\" }"

ServerPackageManager::ServerPackageManager(QObject *parent)
    : QObject(parent)
{
}

ServerPackageManager::~ServerPackageManager()
{
}

void ServerPackageManager::processPackage(const QByteArray &json)
{
    QJson::Parser parser;
    bool valid = false;
    qDebug() << QString("recv data: %1").arg(QString::fromUtf8(json));
    QVariantMap data = parser.parse(json, &valid).toMap();
    if(!valid) {
        qDebug() << QString("error: Invalid packet: %1").arg(QString::fromUtf8(json));
        send(PKG_CMD_ERROR);
        return;
    }

    const QString status = processJson(data);
    data["status"] = status;
    Package pkg;
    pkg.serialize(data);
    send(pkg.json());
}

QString ServerPackageManager::processJson(const QVariantMap &data)
{
    /*
    if(data.contains("radio")) {
        QVariantMap radioMap = data["radio"].toMap();
        const int radioNo = radioMap["radioNo"].toInt();
        QString action = radioMap["action"].toString();

        if(action.compare("connect") == 0) {
            if(!RadiosManager::instance()->connect(radioNo)) {
                return RADIO_NOT_VALID;
            }
            return CMD_OK;
        } else if(action.compare("disconnect") == 0) {
            if(!RadiosManager::instance()->disconnect(radioNo)) {
                return RADIO_NOT_VALID;
            }
            return CMD_OK;
        } else if(action.compare("ptt-key") == 0) {
            if(!RadiosManager::instance()->pttKey(radioNo)) {
                return RADIO_NOT_VALID;
            }
            return CMD_OK;
        } else if(action.compare("ptt-unkey") == 0) {
            if(!RadiosManager::instance()->pttUnkey(radioNo)) {
                return RADIO_NOT_VALID;
            }
            return CMD_OK;
        } else if(action.compare("volume") == 0) {
            if(radioMap.contains("value")) {
                const int value = radioMap["value"].toInt();
                if(value < 0 || value > 100)
                    return CMD_ERROR;

                if(!RadiosManager::instance()->setVolume(radioNo, value)) {
                    return RADIO_NOT_VALID;
                }
                return CMD_OK;
            }
        } else if(action.compare("mute") == 0) {
            if(radioMap.contains("value")) {
                const QString value = radioMap["value"].toString();
                if(value.compare("speaker") != 0 && value.compare("mic") != 0) {
                    return CMD_ERROR;
                }

                if(!RadiosManager::instance()->mute(radioNo, value)) {
                    return RADIO_NOT_VALID;
                }
                return CMD_OK;
            }
        } else if(action.compare("unmute") == 0) {
            if(radioMap.contains("value")) {
                const QString value = radioMap["value"].toString();
                if(value.compare("speaker") != 0 && value.compare("mic") != 0) {
                    return CMD_ERROR;
                }

                if(!RadiosManager::instance()->unmute(radioNo, value)) {
                    return RADIO_NOT_VALID;
                }
                return CMD_OK;
            }
        }
    } else if(data.contains("radios")) {
        QVariantMap radiosMap = data["radios"].toMap();
        QString action = radiosMap["action"].toString();
        if(action.compare("list") == 0) {
            QList<Radio *> *radiosList = RadiosManager::instance()->radiosList();
            QString reply = "{ \"status\": \"cmd_ok\", ";
            reply += QString("\"count\" : \"%1\", ").arg(radiosList->size());
            reply += QString("\"radios\" : [");
            for(int i = 0; i < radiosList->size(); i++) {
                reply += QString("{ \"radio\" : \"%1\",").arg(i);
                reply += QString("   \"name\" : \"%1\"").arg(radiosList->at(i)->name());
                reply += (radiosList->size() == i+1) ? "} " : "}, ";
            }
            reply += "]";
            reply += "}";
            return reply;
        }
    }
*/

    return CMD_ERROR;
}

void ServerPackageManager::createPackage(const int radioNo, const QString &type, const QString &value)
{
    QString pkg;
    if(type.compare("UnitID") == 0) {
        pkg = "{ \"radios\": {";
        pkg += QString("\"radioNo\" : \"%1\", ").arg(radioNo);
        pkg += QString("\"UnitID\" : \"%1\"").arg(value);
        pkg += "}";
        pkg += "}";
    } else {
        return;
    }

    send(pkg);
}

void ServerPackageManager::send(const QString &rep)
{
    m_packageList.append(rep);
    emit readySend();
}
