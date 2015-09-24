/**
 *
 * Author: Samuel Isuani <sisuani@gmail.com>
 *
 */

#include "application.h"

#include <QTimer>
#include <QFile>
#include <QMessageBox>

#include "config.h"
#include "settings.h"
//#include "modules.h"
#include "mainwindow.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

Application::Application(int &argc, char **argv)
  : QApplication(argc, argv)
{
    setOrganizationName(QLatin1String(VOIPC_ORGANIZATION_NAME));
    setOrganizationDomain(QLatin1String(VOIPC_ORGANIZATION_DOMAIN));
    setApplicationName(QLatin1String(VOIPC_APPLICATION_NAME));
    setApplicationVersion(QLatin1String(VOIPC_VERSION));

    QApplication::setQuitOnLastWindowClosed(true);

    QTimer::singleShot(0, this, SLOT(init()));
}

Application::~Application()
{
}

Application *Application::instance()
{
    return (static_cast<Application*>(QCoreApplication::instance()));
}

void Application::init()
{
    // settings
    if(!initSettings())
        return;

    // gui
    initGui();
    qApp->processEvents();
}

bool Application::initSettings()
{
    QSettings::setDefaultFormat(QSettings::IniFormat);
    if(!Settings::instance()->isValid())
        return false;
    return true;
}

void Application::initGui()
{
    mMainWindow = new MainWindow(0);
    mainWindow()->show();
}
