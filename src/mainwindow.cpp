
#include "mainwindow.h"

#include <QMessageBox>
#include <QDebug>

#include "commands.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->d1Button, SIGNAL(clicked()), SLOT(dialButtonClicked()));
    connect(ui->d2Button, SIGNAL(clicked()), SLOT(dialButtonClicked()));
    connect(ui->d3Button, SIGNAL(clicked()), SLOT(dialButtonClicked()));
    connect(ui->d4Button, SIGNAL(clicked()), SLOT(dialButtonClicked()));
    connect(ui->d5Button, SIGNAL(clicked()), SLOT(dialButtonClicked()));
    connect(ui->d6Button, SIGNAL(clicked()), SLOT(dialButtonClicked()));
    connect(ui->d7Button, SIGNAL(clicked()), SLOT(dialButtonClicked()));
    connect(ui->d8Button, SIGNAL(clicked()), SLOT(dialButtonClicked()));
    connect(ui->d9Button, SIGNAL(clicked()), SLOT(dialButtonClicked()));
    connect(ui->d0Button, SIGNAL(clicked()), SLOT(dialButtonClicked()));
    connect(ui->dStarButton, SIGNAL(clicked()), SLOT(dialButtonClicked()));
    connect(ui->dNumeralButton, SIGNAL(clicked()), SLOT(dialButtonClicked()));

    connect(ui->muteButton, SIGNAL(clicked()), SLOT(muteClicked()));

    connect(ui->callButton, SIGNAL(clicked()), SLOT(callClicked()));
    connect(ui->hangupButton, SIGNAL(clicked()), SLOT(hangupClicked()));
    connect(&voipc, SIGNAL(stateChanged()), SLOT(voipCStateChanged()));

    initialize();

    muteClicked();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initialize()
{
    m_incommingCall = false;

    if (voipc.registered())
        voipc.shutdown();
    setStatus(trUtf8("Inicializando SIP..."));
    ui->statusLabel->setText("Inicializando SIP...");
    if (!voipc.initialize())
        setStatus(trUtf8("Error inicializando"));
    else
        setStatus(trUtf8("Registrado"));

}

void MainWindow::dialButtonClicked()
{
    QPushButton *senderButton = (QPushButton *) sender();
    if(!senderButton)
        return;

    const QString dtxt = senderButton->text();
    ui->uriComboBox->setCurrentText(ui->uriComboBox->currentText() + dtxt);
}

void MainWindow::callClicked()
{
    if (!voipc.registered()) {
        QMessageBox::warning(this,
                trUtf8("VoipC"),
                trUtf8("No está registrado"));
        return;
    }

    if (voipc.state() == "INCOMING") {
        voipc.answer();
    } else if(voipc.state() == "DISCONNCTD") {
        if (!voipc.call(ui->uriComboBox->currentText())) {
            setStatus(trUtf8("Error realizando la llamada"));
        }
    }
}

void MainWindow::hangupClicked()
{
    if(!voipc.registered()) {
        QMessageBox::warning(this,
                trUtf8("VoipC"),
                trUtf8("No está registrado"));
        return;
    }

    // voipc.hangup(numero_de_llamada) -> Imp
    voipc.hangup();
}

void MainWindow::muteClicked()
{
    if (ui->muteButton->isChecked())
        voipc.setRxLevel(0, 0);
    else
        voipc.setRxLevel(0, 2);
}

void MainWindow::voipCStateChanged()
{
    const QString state = voipc.state();
    qDebug() << "current state: " << state;
    if(state.compare("INCOMING") == 0) {
        m_incommingCall = true;
        ui->callButton->setText(trUtf8("Atender"));
    } else if (state.compare("CONNECTING") == 0) {
        setStatus(trUtf8("Conectando..."));
    } else if (state.compare("CONFIRMED") == 0) {
        setStatus(trUtf8("Conectado"));
    } else if (state.compare("DISCONNCTD") == 0) {
        if(m_incommingCall) {
            m_incommingCall = false;
            ui->callButton->setText(trUtf8("Llamar"));
        }
        setStatus(trUtf8("En espera..."));
    }
}

void MainWindow::setStatus(const QString &status)
{
    ui->statusLabel->setText(status);
}

void MainWindow::execCommand(const int cmd, const QString &arg)
{
    if (cmd == Dial) {
        QString d;
        if (arg == "*")
            d = "Star";
        else if (arg == "#")
            d = "Number";

        QPushButton *dButton = findChild<QPushButton *>(QString::fromUtf8("d%1Button").arg(d));
        if (dButton)
            dButton->clicked();
    } else if (cmd == Call) {
        ui->uriComboBox->setCurrentText(arg);
        callClicked();
    } else if (cmd == Hangup) {
        hangupClicked();
    } else if (cmd == Pickup) {
        if (voipc.state() == "INCOMING") {
            callClicked();
        }
    } else if (cmd == Mute) {
        ui->muteButton->setChecked(true);
        muteClicked();
    } else if (cmd == Unmute) {
        ui->muteButton->setChecked(false);
        muteClicked();
    }

}
