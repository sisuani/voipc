/**
 *
 * Author: Samuel Isuani <sisuani@gmail.com>
 *
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

class Settings : public QObject
{
Q_OBJECT

public:
    static Settings *instance();

    Settings(QObject *parent = 0);
    ~Settings();

    bool isValid();

    // sip
    void setSipDomain(const QString &sipDomain);
    const QString &sipDomain() const;

    void setSipUserName(const QString &sipUserName);
    const QString &sipUserName() const;

    void setSipPassword(const QString &sipPassword);
    const QString &sipPassword() const;


private:
    void readSettings();

    QSettings qsettings;
    QString m_sipDomain;
    QString m_sipUserName;
    QString m_sipPassword;

    static Settings *m_instance;
};

#endif // SETTINGS_H
