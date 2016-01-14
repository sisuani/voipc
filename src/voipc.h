/*
 *
 */


#ifndef VOIPC_H
#define VOIPC_H

#include <QObject>

#include "PjCallback.h"

class VoipC : public QObject
{
Q_OBJECT

public:
    VoipC(QObject *parent = 0);
    ~VoipC();

    int regStatus();
    bool initialize();
    bool unregister();
    bool shutdown();
    bool call(const QString &uri);
    bool hangup();
    bool answer();
    bool hold(const bool hold);
    bool sendDtmf(char digit);
    void setTxLevel(int slot, float level);
    void setRxLevel(int slot, float level);

    const QString &state() const;
    const QString &statusContact() const;
    const QString &reason() const;

signals:
    void stateChanged();

private slots:
    void setCallState(const QString &state, const QString &contact, const QString &reason);

private:
    PjCallback *pjCallback;
    QString m_state;
    QString m_status_contact;
    QString m_reason;
    char *caor, *creguri, *cdomain, *cusername, *cpassword, *cstun, *coutbound;
};

#endif // VOIPC_H
