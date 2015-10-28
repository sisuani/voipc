/**
 *
 *
 * Author: Samuel Isuani <sisuani@gmail.com>
 *
 */

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QWeakPointer>

#include "settings.h"
#include "network/server.h"

class MainWindow;

class Application : public QApplication
{
Q_OBJECT

public:
    Application(int &argc, char **argv);
    ~Application();
    static Application *instance();

    MainWindow *mainWindow() const { return mMainWindow; }
    Server *server() const { return mServer; }

private slots:
    void init();

private:
    bool initSettings();
    void initGui();

    Settings *settings;
    Server *mServer;
    MainWindow *mMainWindow;
};

#endif // APPLICATION_H
