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
#include "application.h"
#include "mainwindow.h"
#include "commands.h"

#define CMD_OK          "cmd_ok"
#define CMD_ERROR       "cmd_error"
#define PKG_CMD_ERROR   "{ \"status\": \"cmd_error\" }"

#define CMD_DIAL        "dial"
#define CMD_CALL        "call"
#define CMD_HANGUP      "hangup"
#define CMD_PICKUP      "pickup"
#define CMD_MUTE        "mute"
#define CMD_UNMUTE      "unmute"
#define CMD_GET_STATUS  "get_status"
#define CMD_MUTE_STATUS "get_mute_status"


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
    if(!data.contains("action"))
        return CMD_ERROR;

    const QString action = data["action"].toString();

    if (action.compare(CMD_DIAL) == 0) {
        if (data.contains("digit")) {
            Application::instance()->mainWindow()->execCommand(Dial, data["digit"].toString());
            return CMD_OK;
        }
        return CMD_ERROR;
    } else if (action.compare(CMD_CALL) == 0) {
        if (data.contains("dst")) {
            Application::instance()->mainWindow()->execCommand(Call, data["dst"].toString());
            return CMD_OK;
        }
        return CMD_ERROR;
    } else if (action.compare(CMD_HANGUP) == 0) {
        Application::instance()->mainWindow()->execCommand(Hangup, "");
        return CMD_OK;
    } else if (action.compare(CMD_PICKUP) == 0) {
        Application::instance()->mainWindow()->execCommand(Pickup, "");
        return CMD_OK;
    } else if (action.compare(CMD_MUTE) == 0) {
        Application::instance()->mainWindow()->execCommand(Mute, "");
        return CMD_OK;
    } else if (action.compare(CMD_UNMUTE) == 0) {
        Application::instance()->mainWindow()->execCommand(Unmute, "");
        return CMD_OK;
    }

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
