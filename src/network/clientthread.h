/*
 *
 *
 * Author: Samuel Isuani <sisuani@gmail.com>
 *
 *
 */

#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QMutex>
#include <QTimer>

#include "serverpackagemanager.h"

class ClientThread : public QObject
{
Q_OBJECT

public:
    ClientThread(int socket);

signals:
    void finished();

private slots:
    void disconnected();
    void process();
    void readyRead();
    void readySend();

private:
    int m_socket;
    QMutex m_mutex;
    QTcpSocket *m_tcpSocket;
    ServerPackageManager m_serverPackageManager;
};

#endif // CLIENTTHREAD_H
