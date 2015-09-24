/*
 *
 *
 * Author: Samuel Isuani <sisuani@gmail.com>
 *
 *
 */

#include "clientthread.h"

#include <QTcpSocket>
#include <QDebug>

ClientThread::ClientThread(int socket)
{
    m_socket = socket;
    qDebug() << QString("New client sk: %1 ").arg(socket);
}

void ClientThread::process()
{
    m_tcpSocket = new QTcpSocket();
    if (!m_tcpSocket->setSocketDescriptor(m_socket))
        return;

    connect(&m_serverPackageManager, SIGNAL(readySend()), SLOT(readySend()));
    connect(m_tcpSocket, SIGNAL(readyRead()), SLOT(readyRead()));
    connect(m_tcpSocket, SIGNAL(disconnected()), SLOT(disconnected()));
}

void ClientThread::readyRead()
{
    if(m_tcpSocket->canReadLine()) {
        m_serverPackageManager.processPackage(m_tcpSocket->readLine());
    } else {
        //m_tcpSocket->readAll();
    }
}

void ClientThread::readySend()
{
    while(m_serverPackageManager.hasPackage()) {
        if(m_tcpSocket->state() != QAbstractSocket::ConnectedState)
            return;

        //m_tcpSocket->write(m_serverPackageManager.first().json() + '\n');
        if(m_tcpSocket->waitForBytesWritten(3000)) {
            m_serverPackageManager.pop_front();
        } else {
            //disconnected();
        }
    }
    readyRead();
}

void ClientThread::disconnected()
{
    m_tcpSocket->close();
    emit finished();
}
