/*
 *
 *
 * Author: Samuel Isuani <sisuani@gmail.com>
 *
 *
 */

#ifndef SERVERPACKAGEMANAGER_H
#define SERVERPACKAGEMANAGER_H

#include <QObject>
#include <QVector>
#include <QVariantMap>

class ServerPackageManager : public QObject
{
Q_OBJECT

public:
    ServerPackageManager(QObject *parent = 0);
    ~ServerPackageManager();

    void processPackage(const QByteArray &json);
    void createPackage(const int radioNo, const QString &type, const QString &value);

    inline bool hasPackage() { return m_packageList.count() > 0; }
    inline bool isEmpty() { return m_packageList.isEmpty(); }
    inline QString first() { return m_packageList.first(); }
    inline void pop_front() { m_packageList.pop_front(); }

signals:
    void readySend();

private:
    QString processJson(const QVariantMap &data);
    void send(const QString &rep);
    QVector<QString> m_packageList;
};

#endif // SERVERPACKAGEMANAGER_H
