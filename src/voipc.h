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
    bool shutdown();
    bool call(const QString &uri);
    bool hangup();
    bool answer();
    const QString &state() const;
    const QString &statusContact() const;
    void setTxLevel(int slot, float level);
    void setRxLevel(int slot, float level);

signals:
    void stateChanged();

private slots:
    void setCallState(const QString &state, const QString &contact);

private:
    PjCallback *pjCallback;
    QString m_state;
    QString m_status_contact;
    char *caor, *creguri, *cdomain, *cusername, *cpassword, *cstun, *coutbound;
};

#endif // VOIPC_H
