
#include "voipc.h"

#include <QDebug>
#include <QMutex>
#include <QMessageBox>

#include "application.h"
#include "settings.h"

extern "C" {
#include <pjlib.h>
#include <pjlib-util.h>
#include <pjmedia.h>
#include <pjmedia-codec.h>
#include <pjsip.h>
#include <pjsip_simple.h>
#include <pjsip_ua.h>
#include <pjsua-lib/pjsua.h>
}

extern void *globalPjCallback;
QList<int> activeCalls;
QMutex activeCallsMutex;

pjsua_config cfg;
pjsua_logging_config log_cfg;
pjsua_media_config media_cfg;
pjsua_transport_config transport_cfg;
pjsua_transport_config rtp_cfg;
pjsua_buddy_config buddy_cfg;
pjsua_acc_config acc_cfg;
pjsua_acc_id acc_id;
pj_pool_t *pool;


static pj_bool_t default_mod_on_rx_request(pjsip_rx_data *rdata)
{
    pjsip_tx_data *tdata;
    pjsip_status_code status_code;
    pj_status_t status;

    if(pjsip_method_cmp(&rdata->msg_info.msg->line.req.method,
                &pjsip_ack_method) == 0)
        return PJ_TRUE;

    if(pjsip_method_cmp(&rdata->msg_info.msg->line.req.method,
                &pjsip_notify_method) == 0) {
        status_code = PJSIP_SC_BAD_REQUEST;
    } else {
        status_code = PJSIP_SC_METHOD_NOT_ALLOWED;
    }
    status = pjsip_endpt_create_response(pjsua_get_pjsip_endpt(), rdata,
            status_code, NULL, &tdata);
    if(status != PJ_SUCCESS) {
        qDebug() << "Unable to create response" << status;
        return PJ_TRUE;
    }

    if(status_code == PJSIP_SC_METHOD_NOT_ALLOWED) {
        const pjsip_hdr *cap_hdr;
        cap_hdr = pjsip_endpt_get_capability(pjsua_get_pjsip_endpt(),
                PJSIP_H_ALLOW, NULL);
        if(cap_hdr) {
            pjsip_msg_add_hdr(tdata->msg, (pjsip_hdr*)pjsip_hdr_clone(tdata->pool, cap_hdr));
        }
    }

    /* Add User-Agent header */
    {
        pj_str_t user_agent;
        char tmp[255];
        const pj_str_t USER_AGENT_HDR = {"User-Agent", 10};
        pjsip_hdr *h;

        pj_ansi_snprintf(tmp, sizeof(tmp), "VoipC");
        pj_strdup2_with_null(tdata->pool, &user_agent, tmp);

        h = (pjsip_hdr*) pjsip_generic_string_hdr_create(tdata->pool,
                &USER_AGENT_HDR,
                &user_agent);
        pjsip_msg_add_hdr(tdata->msg, h);
    }

    pjsip_endpt_send_response2(pjsua_get_pjsip_endpt(), rdata, tdata, NULL,
            NULL);

    return PJ_TRUE;
}

static pjsip_module mod_default_handler =
{
    NULL, NULL,				/* prev, next.		*/
    { "mod-default-handler", 19 },	/* Name.		*/
    -1,					/* Id			*/
    PJSIP_MOD_PRIORITY_APPLICATION+99,	/* Priority	        */
    NULL,				/* load()		*/
    NULL,				/* start()		*/
    NULL,				/* stop()		*/
    NULL,				/* unload()		*/
    default_mod_on_rx_request,		/* on_rx_request()	*/
    NULL,				/* on_rx_response()	*/
    NULL,				/* on_tx_request.	*/
    NULL,				/* on_tx_response()	*/
    NULL,				/* on_tsx_state()	*/

};

VoipC::VoipC(QObject *parent)
{
    pjCallback = new PjCallback();
    m_state = "DISCONNCTD";

   // QObject::connect((PjCallback*)globalPjCallback, SIGNAL(new_log_message(QString)),
   //         this, SLOT(dump_log_message(QString)), Qt::QueuedConnection);
   //  QObject::connect((PjCallback*)globalPjCallback, SIGNAL(new_im(QString,QString)),
   //          this, SLOT(new_incoming_im(QString,QString)), Qt::QueuedConnection);
    connect((PjCallback*)globalPjCallback, SIGNAL(setCallState(const QString &, const QString &)),
            SLOT(setCallState(const QString &, const QString &)), Qt::QueuedConnection);
    //QObject::connect((PjCallback*)globalPjCallback, SIGNAL(setCallButtonText(QString)),
            //Application::instance()->mainWindow(), SLOT(setCallButtonText(QString)), Qt::QueuedConnection);
    /*
    QObject::connect((PjCallback*)globalPjCallback, SIGNAL(buddy_state(int)),
            this, SLOT(buddy_state(int)), Qt::QueuedConnection);
    QObject::connect((PjCallback*)globalPjCallback, SIGNAL(reg_state_signal(int)),
            this, SLOT(reg_state_slot(int)), Qt::QueuedConnection);
    QObject::connect((PjCallback*)globalPjCallback, SIGNAL(nat_detect(QString,QString)),
            this, SLOT(nat_detect_slot(QString,QString)), Qt::QueuedConnection);
            */

    caor=creguri=cdomain=cusername=cpassword=cstun=coutbound=0;
}

VoipC::~VoipC()
{
}

bool VoipC::initialize()
{
    QByteArray tempStun;
    pj_status_t status;

    const QString domain = Settings::instance()->sipDomain();
    const QString username = Settings::instance()->sipUserName();
    const QString password = Settings::instance()->sipPassword();

    if (domain.isEmpty() || username.isEmpty() ) {
        QMessageBox::warning(0,
                trUtf8("VoipC"),
                trUtf8("Debe completar dominio y nombre de usuario"));
        return false;
    }

    status = pjsua_create();
    if(status != PJ_SUCCESS) {
        qDebug() << "Error in pjsua_create()" << status;
        pjsua_destroy();
        return false;
    }

    pool = pjsua_pool_create("voipc", 1000, 1000);

    pjsua_config_default(&cfg);
    unsigned int count = 0;
    cfg.nameserver_count=0;
    for (count=0;count<4;count++) {
        if (cfg.nameserver[count].ptr) {
            free(cfg.nameserver[count].ptr);
            cfg.nameserver[count].ptr = 0;
            cfg.nameserver[count].slen = 0;
        }
    }

    cfg.cb.on_incoming_call = PjCallback::on_incoming_call_wrapper;
    cfg.cb.on_call_media_state = PjCallback::on_call_media_state_wrapper;
    cfg.cb.on_call_state = PjCallback::on_call_state_wrapper;
    cfg.cb.on_pager = PjCallback::on_pager_wrapper;
    cfg.cb.on_reg_state = PjCallback::on_reg_state_wrapper;
    cfg.cb.on_buddy_state = PjCallback::on_buddy_state_wrapper;
    cfg.cb.on_nat_detect = PjCallback::on_nat_detect_wrapper;
    const char tmp[256] = "VoipC";
    pj_strdup2_with_null(pool, &(cfg.user_agent), tmp);

    pjsua_logging_config_default(&log_cfg);
    log_cfg.msg_logging = true;
    log_cfg.console_level = 3;
    log_cfg.cb = PjCallback::logger_cb_wrapper;
    log_cfg.decor = log_cfg.decor & ~PJ_LOG_HAS_NEWLINE;

    pjsua_media_config_default(&media_cfg);
    media_cfg.no_vad = true;
    status = pjsua_init(&cfg, &log_cfg, &media_cfg);
    if(status != PJ_SUCCESS) {
        qDebug() << "Error in pjsua_init()" << status;
        pjsua_destroy();
        return false;
    }

    status = pjsip_endpt_register_module(pjsua_get_pjsip_endpt(),
            &mod_default_handler);


    pjsua_transport_id transport_id;
    pjsua_transport_config_default(&transport_cfg);
    transport_cfg.port = 0;
    QString transport = "UDP";
    if (transport == "TCP") {
        status = pjsua_transport_create(PJSIP_TRANSPORT_TCP, &transport_cfg, &transport_id);
#if defined(PJ_HAS_IPV6) && PJ_HAS_IPV6
    } else if (transport == "UDP6") {
        status = pjsua_transport_create(PJSIP_TRANSPORT_UDP6, &transport_cfg, &transport_id);
    } else if (transport == "TCP6") {
        status = pjsua_transport_create(PJSIP_TRANSPORT_TCP6, &transport_cfg, &transport_id);
#endif
    } else if (transport == "TLS") {
        transport_cfg.tls_setting.method = PJSIP_TLSV1_METHOD;
        /*
        transport_cfg.tls_setting.verify_server = tlsVerifyServer;
        QByteArray tempca, tempkey, tempcert;
        tempca=caFile.toLatin1();
        transport_cfg.tls_setting.ca_list_file = pj_str(tempca.data());
        tempkey=privKeyFile.toLatin1();
        transport_cfg.tls_setting.privkey_file = pj_str(tempkey.data());
        tempcert=certFile.toLatin1();
        transport_cfg.tls_setting.cert_file = pj_str(tempcert.data());
        status = pjsua_transport_create(PJSIP_TRANSPORT_TLS, &transport_cfg, &transport_id);
        */
    } else {
        status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &transport_cfg, &transport_id);
    }

    if (status != PJ_SUCCESS) {
        qDebug() << "Error creating transport" << status;
        pjsua_destroy();
        return false;
    }

    pjsua_acc_config_default(&acc_cfg);
    QString sipAoR, regUri, outboundUri;
    sipAoR = QString("sip:") + username + QString("@") + domain;
    regUri = QString("sip:") + domain;
    const QString outbound = "";
    if (!outbound.isEmpty()) {
        outboundUri = QString("sip:") + outbound;
    } else {
        outboundUri = QString("sip:") + domain + ";lr";
        if ( (transport == "UDP") || (transport == "UDP6") )  {
            outboundUri = outboundUri + ";transport=udp";
        } else if ( (transport == "TCP") || (transport == "TCP6") )  {
            outboundUri = outboundUri + ";transport=tcp";
        } else if ( (transport == "TLS") )  {
            outboundUri = outboundUri + ";transport=tls";
        }
    }


    QByteArray temp;
    free(caor);      temp=sipAoR.toLatin1();        caor = strdup(temp.data());
    free(creguri);   temp = regUri.toLatin1();      creguri = strdup(temp.data());
    free(cdomain);   temp = domain.toLatin1();      cdomain = strdup(temp.data());
    free(cusername); temp = username.toLatin1();    cusername = strdup(temp.data());
    free(cpassword); temp = password.toLatin1();    cpassword = strdup(temp.data());
    free(coutbound); temp = outboundUri.toLatin1(); coutbound = strdup(temp.data());

    acc_cfg.id = pj_str(caor);
    acc_cfg.reg_uri = pj_str(creguri);
    acc_cfg.cred_count = 1;
    acc_cfg.cred_info[0].realm = pj_str("*");
    acc_cfg.cred_info[0].scheme = pj_str("digest");
    acc_cfg.cred_info[0].username = pj_str(cusername);
    acc_cfg.cred_info[0].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
    acc_cfg.cred_info[0].data = pj_str(cpassword);
    acc_cfg.proxy_cnt = 1;
    acc_cfg.proxy[0] = pj_str(coutbound);
    acc_cfg.publish_enabled = PJ_TRUE;

    const QString srtp = "disabled";
    if (srtp == "disabled") {
        acc_cfg.use_srtp = PJMEDIA_SRTP_DISABLED;
    } else if (srtp == "optional") {
        acc_cfg.use_srtp = PJMEDIA_SRTP_OPTIONAL;
    } else if (srtp == "mandatory") {
        acc_cfg.use_srtp = PJMEDIA_SRTP_MANDATORY;
    }
    acc_cfg.srtp_secure_signaling = 0;
    acc_cfg.allow_contact_rewrite = 0;

    status = pjsua_acc_add(&acc_cfg, PJ_TRUE, &acc_id);
    if(status != PJ_SUCCESS) {
        QMessageBox::warning(0,
                tr("VoipC"),
                tr("No se puede registrar"));
        shutdown();
        return false;
    }

    status = pjsua_start();
    if (status != PJ_SUCCESS) {
        qDebug() << "Error starting pjsua" << status;
        pjsua_destroy();
        return false;
    }

    return true;
}

bool VoipC::shutdown()
{
    pj_status_t status;
    if (pool) {
        pj_pool_release(pool);
        pool = NULL;
    }

    status = pjsua_destroy();
    if (status != PJ_SUCCESS) {
        qDebug() << "Error destroying pjsua" << status;
        return false;
    }

    return true;
}

bool VoipC::call(const QString &uri)
{
    activeCallsMutex.lock();
    if (!activeCalls.empty()) {
        activeCallsMutex.unlock();
        return false;
    }

    QString tempstring;
    QByteArray temparray1 =tempstring.toLatin1();
    pj_str_t hname1 = pj_str(temparray1.data());
    QByteArray temparray2 =tempstring.toLatin1();
    pj_str_t hname2 = pj_str(temparray2.data());
    QByteArray temparray3 =tempstring.toLatin1();
    pj_str_t hval1 = pj_str(temparray3.data());
    QByteArray temparray4 =tempstring.toLatin1();
    pj_str_t hval2 = pj_str(temparray4.data());
    pjsua_msg_data msg_data;
    pjsip_generic_string_hdr header1;
    pjsip_generic_string_hdr header2;
    pjsua_msg_data_init(&msg_data);

    int call_id;
    QByteArray temp = uri.toLatin1();
    pj_str_t pjuri = pj_str(temp.data());
    pj_status_t status = pjsua_call_make_call(acc_id, &pjuri, 0, 0, &msg_data, &call_id);
    bool ret = false;
    if(status != PJ_SUCCESS) {
        qDebug() << "Error calling buddy" << status;
    } else {
        ret = true;
        activeCalls << call_id;
    }
    activeCallsMutex.unlock();

    return ret;
}

bool VoipC::hangup()
{
    activeCallsMutex.lock();
    if(activeCalls.empty()) {
        activeCallsMutex.unlock();
        return false;
    }
    pjsua_call_hangup(activeCalls.at(0),0,0,0);
    activeCallsMutex.unlock();
    return true;
}

bool VoipC::answer()
{
    activeCallsMutex.lock();
    if (activeCalls.empty()) {
        activeCallsMutex.unlock();
        return false;
    }
    pjsua_call_answer(activeCalls.at(0),200,0,0);
    activeCallsMutex.unlock();
}

bool VoipC::hold(const bool hold)
{
    if (activeCalls.empty())
        return false;

    if (hold) {
        pj_status_t status = pjsua_call_set_hold(activeCalls.at(0), 0);
        if (status != PJ_SUCCESS) {
            return false;
        }
    } else {
        pj_status_t status = pjsua_call_reinvite(activeCalls.at(0), true, 0);
        if (status != PJ_SUCCESS) {
            return false;
        }
    }
    return true;
}

bool VoipC::sendDtmf(char digit)
{
    if (activeCalls.empty())
        return false;

    pj_str_t s = pj_str(&digit);
    pj_status_t status = pjsua_call_dial_dtmf(activeCalls.at(0), &s);
    if (status != PJ_SUCCESS)
        return false;

    return true;
}

int VoipC::regStatus()
{
    pj_status_t status;
    pjsua_acc_info acc_info;
    status = pjsua_acc_get_info(acc_id, &acc_info);

    return acc_info.status;
}

void VoipC::setCallState(const QString &state, const QString &contact)
{
    m_state = state;
    m_status_contact = contact;

    emit stateChanged();
}

const QString &VoipC::state() const
{
    return m_state;
}

const QString &VoipC::statusContact() const
{
    return m_status_contact;
}

void VoipC::setTxLevel(int slot, float level)
{
    pjsua_conf_adjust_tx_level(slot, level);
}

void VoipC::setRxLevel(int slot, float level)
{
    pjsua_conf_adjust_rx_level(slot, level);
}
