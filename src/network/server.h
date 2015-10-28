/*
 *
 *
 * Author: Samuel Isuani <sisuani@gmail.com>
 *
 *
 */

#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTimer>

#include "clientthread.h"

class Server : public QTcpServer
{
Q_OBJECT

public:
    Server(quint16 port, QObject* parent = 0);
    ~Server();

    void sendToAll(const QString &cmd, const QString &args);

private slots:
    void deleteClient();

protected:
    void incomingConnection(qintptr socket);
    QList<ClientThread *> clientsList;
};

#endif // SERVER_H
