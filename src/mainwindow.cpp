
#include "mainwindow.h"

#include <QMessageBox>
#include <QTimer>
#include <QLineEdit>
#include <QDebug>
#include <QDir>
#include <QShortcut>

#include "application.h"
#include "settings.h"
#include "commands.h"

#define SS_ERROR "color: #E74C3C;"
#define SS_WAIT  "color: #F39C12;"
#define SS_OK    "color: #2ECC71;"
#define SS_NULL  ""

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , toneSound(0)
    , ringSound(0)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() ^ Qt::WindowMaximizeButtonHint);

    const QString qdircp = QDir::currentPath() + QDir::separator();
    toneSound = new QSound(qdircp + "tone.wav");
    ringSound = new QSound(qdircp + "ring.wav");
    toneSound->setLoops(-1);
    ringSound->setLoops(-1);

    QShortcut *callShortcut = new QShortcut(QKeySequence("Ctrl+L"), this);
    QShortcut *hangupShortcut = new QShortcut(QKeySequence("Ctrl+H"), this);

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
    connect(ui->holdButton, SIGNAL(clicked()), SLOT(holdClicked()));

    connect(ui->uriComboBox->lineEdit(), SIGNAL(returnPressed()), SLOT(callClicked()));

    connect(ui->callButton, SIGNAL(clicked()), SLOT(callClicked()));
    connect(callShortcut, SIGNAL(activated()), SLOT(callClicked()));

    connect(ui->hangupButton, SIGNAL(clicked()), SLOT(hangupClicked()));
    connect(hangupShortcut, SIGNAL(activated()), SLOT(hangupClicked()));

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


void MainWindow::closeEvent(QCloseEvent* event)
{
    voipc.shutdown();
}

void MainWindow::initialize()
{
    m_incommingCall = false;

    setStatus(trUtf8("Inicializando SIP..."), SS_WAIT);
    if (!voipc.initialize()) {
        setStatus(trUtf8("Error inicializando"), SS_ERROR);
     } else {
        setStatus(trUtf8("Registrando..."), SS_WAIT);
        QTimer::singleShot(10, this, SLOT(checkRegistration()));
     }

}

void MainWindow::dialButtonClicked()
{
    QPushButton *senderButton = (QPushButton *) sender();
    if(!senderButton)
        return;

    const QString dtxt = senderButton->text();
    const int cp = ui->uriComboBox->lineEdit()->cursorPosition();
    QString ct = ui->uriComboBox->currentText();
    ct.insert(cp, dtxt);
    ui->uriComboBox->setCurrentText(ct);
    ui->uriComboBox->lineEdit()->setCursorPosition(cp+1);

    const QString state = voipc.state();

    if (state.compare("CONNECTING") == 0 || state.compare("CONFIRMED") == 0) {
        sendDtmf(dtxt.at(0).toLatin1());
    }
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
        QString dest = ui->uriComboBox->currentText();
        if (!dest.startsWith("sip:"))
            dest.insert(0, "sip:");

        if (!dest.contains("@"))
            dest.append("@" + Settings::instance()->sipDomain());

        if (!voipc.call(dest)) {
            setStatus(trUtf8("Error llamando a <%1>").arg(dest), SS_ERROR);
        } else {
            setStatus(trUtf8("Llamando a: <%1>").arg(dest), SS_OK);
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
    const bool c = ui->muteButton->isChecked();
    voipc.setRxLevel(0, c ? 0 : 2);

    if (Settings::instance()->serverEnable())
        Application::instance()->server()->sendToAll("mute_state", c ? "mute" : "unmute");
}

void MainWindow::holdClicked()
{
    if (ui->holdButton->isChecked()) {
        const bool r = voipc.hold(true);
        ui->holdButton->setChecked(r);
    } else {
        const bool r = voipc.hold(false);
        ui->holdButton->setChecked(!r);
    }
}

void MainWindow::voipCStateChanged()
{
    const QString state = voipc.state();
    if(state.compare("INCOMING") == 0) {
        incommingCall();
    } else if (state.compare("CONNECTING") == 0) {
        setStatus(trUtf8("Estableciendo..."), SS_WAIT);
    } else if (state.compare("CONFIRMED") == 0) {
        confirmCall();
    } else if (state.compare("DISCONNCTD") == 0) {
        disconnctdCall();
    }
}

void MainWindow::incommingCall()
{
    m_incommingCall = true;
    ui->callButton->setText(trUtf8("Atender"));
    ringSound->play();
    setStatus(trUtf8("Llamada entrante: %1").arg(voipc.statusContact()), SS_NULL);

    if (Settings::instance()->serverEnable()) {
        QString contact = voipc.statusContact();
        contact.remove("<sip:");
        contact.remove(contact.indexOf("@"), contact.size());

        Application::instance()->server()->sendToAll("incomming_call", contact);
    }
}

void MainWindow::confirmCall()
{
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

    setStatus(trUtf8("Conectado con: %1").arg(condest), SS_NULL);

    if (Settings::instance()->serverEnable())
        Application::instance()->server()->sendToAll("pickup", "");
}

void MainWindow::disconnctdCall()
{
    if (m_incommingCall) {
        m_incommingCall = false;
        ui->callButton->setText(trUtf8("Llamar"));
    }

    if (!toneSound->isFinished())
        toneSound->stop();

    if (!ringSound->isFinished())
        ringSound->stop();

    setStatus(trUtf8("Llamada finalizada"), SS_NULL);

    if (Settings::instance()->serverEnable())
        Application::instance()->server()->sendToAll("hangup", "");
}

void MainWindow::checkRegistration()
{
    const int reg_status = voipc.regStatus();
    if (reg_status == 200)
        setStatus(trUtf8("Registrado"), SS_OK);
    else if (reg_status > 400)
        setStatus(trUtf8("Error registrando"), SS_ERROR);
    else
        QTimer::singleShot(400, this, SLOT(checkRegistration()));
}

void MainWindow::setStatus(const QString &status, const QString &styleSheet)
{
    ui->statusLabel->setText(status);
    ui->statusLabel->setStyleSheet(styleSheet);
}

void MainWindow::sendDtmf(char digit)
{
    voipc.sendDtmf(digit);
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

QString MainWindow::sipStatus()
{
    const QString state = voipc.state();
    if (state.compare("INCOMING") == 0)
        return "incomming_call";
    else if (state.compare("DISCONNCTD") == 0)
        return  "idle";
    else if (state.compare("CONFIRMED") == 0)
        return  "oncall";

    return "unknown";
}
