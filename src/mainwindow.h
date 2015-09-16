#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "PjCallback.h"
#include "ui_mainwindow.h"

class MainWindow : public QMainWindow
{

Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void registerClicked();
    void callClicked();
    void setCallButtonText(QString text);

private:
    Ui::MainWindow *ui;

    void voipcShupdown();
    void voipcInitialize();
    void voidcRegister();

    PjCallback *pjCallback;
    bool registered;
    char *caor, *creguri, *cdomain, *cusername, *cpassword, *cstun, *coutbound;
};

#endif // MAINWINDOW_H
