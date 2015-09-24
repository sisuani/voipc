/**
 *
 * Author: Samuel Isuani <sisuani@gmail.com>
 *
 */

#include "settings.h"

#include <QMessageBox>
#include <QFile>

Settings *Settings::m_instance = 0;

Settings *Settings::instance()
{
    if(!m_instance)
        m_instance = new Settings;
    return m_instance;
}

Settings::Settings(QObject *parent)
    : QObject(parent)
{
    QSettings::setDefaultFormat(QSettings::IniFormat);
    readSettings();
}

Settings::~Settings()
{
    m_instance = 0;
}

void Settings::readSettings()
{
    qsettings.beginReadArray("SIP");
    setSipDomain(qsettings.value("Domain").toString());
    setSipUserName(qsettings.value("UserName").toString());
    setSipPassword(qsettings.value("Password").toString());
    qsettings.endArray();
}

bool Settings::isValid()
{
    if(!QFile::exists(qsettings.fileName())) {
        QMessageBox::warning(0,
                trUtf8("No existe.."),
                trUtf8("No existe el archivo de configuración: %1").arg(qsettings.fileName()));
        return false;
    }
    return true;
}

void Settings::setSipDomain(const QString &sipDomain)
{
    m_sipDomain = sipDomain;
}

const QString &Settings::sipDomain() const
{
    return m_sipDomain;
}

void Settings::setSipUserName(const QString &sipUserName)
{
    m_sipUserName = sipUserName;
}

const QString &Settings::sipUserName() const
{
    return m_sipUserName;
}

void Settings::setSipPassword(const QString &sipPassword)
{
    m_sipPassword = sipPassword;
}

const QString &Settings::sipPassword() const
{
    return m_sipPassword;
}
