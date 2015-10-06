
#include "mainwindow.h"

#include <QMessageBox>
#include <QTimer>
#include <QDebug>

#include "commands.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , toneSound(0)
    , ringSound(0)
{
    ui->setupUi(this);

    toneSound = new QSound("tone.wav");
    ringSound = new QSound("ring.wav");
    toneSound->setLoops(-1);
    ringSound->setLoops(-1);

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
    if (toneSound)
        delete toneSound;

    if (ringSound)
        delete ringSound;

    delete ui;
}

void MainWindow::initialize()
{
    m_incommingCall = false;

    setStatus(trUtf8("Inicializando SIP..."));
    ui->statusLabel->setText("Inicializando SIP...");
    if (!voipc.initialize()) {
        setStatus(trUtf8("Error inicializando"));
     } else {
        setStatus(trUtf8("Registrando..."));
        QTimer::singleShot(10, this, SLOT(checkRegistration()));
     }

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
    if (voipc.regStatus() != 200) {
        QMessageBox::warning(this,
                trUtf8("VoipC"),
                trUtf8("No está registrado"));
        return;
    }

    if (voipc.state() == "INCOMING") {
        voipc.answer();
    } else if(voipc.state() == "DISCONNCTD") {
        const QString dest = ui->uriComboBox->currentText();
        if (!voipc.call(dest)) {
            setStatus(trUtf8("Error llamando a <%1>").arg(dest));
        } else {
            setStatus(trUtf8("Llamando a: <%1>").arg(dest));
            toneSound->play();
        }
    }
}

void MainWindow::hangupClicked()
{
    if(!voipc.regStatus() == 200) {
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
    if(state.compare("INCOMING") == 0) {
        m_incommingCall = true;
        ui->callButton->setText(trUtf8("Atender"));
        setStatus(trUtf8("Llamada entrante: %1").arg(voipc.statusContact()));
        ringSound->play();
    } else if (state.compare("CONNECTING") == 0) {
        setStatus(trUtf8("Estableciendo..."));
    } else if (state.compare("CONFIRMED") == 0) {
        QString condest;
        if (m_incommingCall) {
            condest = voipc.statusContact();
            if (ringSound && !ringSound->isFinished())
                ringSound->stop();
        } else {
            condest = ui->uriComboBox->currentText();
            if (toneSound && !toneSound->isFinished())
                toneSound->stop();
        }

        setStatus(trUtf8("Conectado con: %1").arg(condest));
    } else if (state.compare("DISCONNCTD") == 0) {
        if (m_incommingCall) {
            m_incommingCall = false;
            ui->callButton->setText(trUtf8("Llamar"));
        }

        if (!toneSound->isFinished())
            toneSound->stop();

        if (!ringSound->isFinished())
            ringSound->stop();

        setStatus(trUtf8("Llamada finalizada"));
    }
}

void MainWindow::checkRegistration()
{
    const int reg_status = voipc.regStatus();
    if (reg_status == 200)
        setStatus(trUtf8("Registrado!"));
    else if (reg_status > 400)
        setStatus(trUtf8("Error registrando"));
    else
        QTimer::singleShot(400, this, SLOT(checkRegistration()));
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
