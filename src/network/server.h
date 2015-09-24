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

class Server : public QTcpServer
{
Q_OBJECT

public:
    Server(quint16 port, QObject* parent = 0);

protected:
    void incomingConnection(int socket);
};

#endif // SERVER_H
