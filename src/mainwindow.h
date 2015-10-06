#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSound>

#include "voipc.h"
#include "ui_mainwindow.h"

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void execCommand(const int cmd, const QString &arg);

private slots:
    void dialButtonClicked();
    void callClicked();
    void hangupClicked();
    void muteClicked();
    void voipCStateChanged();
    void checkRegistration();

private:
    void initialize();
    void setStatus(const QString &status);

    QSound *toneSound;
    QSound *ringSound;

    bool m_incommingCall;
    VoipC voipc;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
