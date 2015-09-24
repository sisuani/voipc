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

    bool registered();
    bool initialize();
    bool shutdown();
    bool call(const QString &uri);
    bool hangup();
    bool answer();
    const QString &state() const;

signals:
    void stateChanged();

private slots:
    void setCallState(const QString &state);

private:
    PjCallback *pjCallback;
    QString m_state;
    bool m_registered;
    char *caor, *creguri, *cdomain, *cusername, *cpassword, *cstun, *coutbound;
};

#endif // VOIPC_H
