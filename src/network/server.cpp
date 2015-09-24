/*
 *
 *
 * Author: Samuel Isuani <sisuani@gmail.com>
 *
 *
 */

#include "server.h"

#include "clientthread.h"

Server::Server(quint16 port, QObject* parent)
    : QTcpServer(parent)
{
    listen(QHostAddress::Any, port);
}

void Server::incomingConnection(int socket)
{
    QThread *thread = new QThread();
    ClientThread *clientThread = new ClientThread(socket);
    connect(thread, SIGNAL(started()), clientThread, SLOT(process()));
    connect(clientThread, SIGNAL(finished()), thread, SLOT(quit()));
    connect(thread, SIGNAL(finished()), clientThread, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
}