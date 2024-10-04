
#include "oscreceiver.h"

#include <QThread>
#include <bitset>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "control/pollingcontrolproxy.h"
#include "osc/ip/UdpSocket.h"
#include "osc/osc/OscPacketListener.h"
#include "osc/osc/OscReceivedElements.h"
#include "oscfunctions.h"

void OscFunctionsSendPtrType(UserSettingsPointer m_pConfig,
        const QString& OscGroup,
        const QString& OscKey,
        enum DefOscBodyType OscBodyType,
        const QString& OscMessageBodyQString,
        int OscMessageBodyInt,
        double OscMessageBodyDouble,
        float OscMessageBodyFloat);

class OscReceivePacketListener : public osc::OscPacketListener {
  public:
    UserSettingsPointer m_pConfig;
    OscReceivePacketListener(UserSettingsPointer aPointerHerePlease) {
        m_pConfig = aPointerHerePlease;
    };

  private:
    void ProcessMessage(const osc::ReceivedMessage& oscMessage,
            const IpEndpointName& remoteEndpoint) {
        (void)remoteEndpoint;
        try {
            if (m_pConfig->getValue<bool>(ConfigKey("[OSC]", "OscSendSyncTriggers"))) {
                sendOscSyncTriggers(m_pConfig);
            }
            processOscMessage(oscMessage);
        } catch (const osc::Exception& e) {
            qDebug() << "OSC Error parsing Msg from "
                     << oscMessage.AddressPattern() << " error: " << e.what();
        }
    }

    void processOscMessage(const osc::ReceivedMessage& message) {
        oscResult oscIn;
        osc::ReceivedMessageArgumentStream args = message.ArgumentStream();
        args >> oscIn.oscValue >> osc::EndMessage;
        oscIn.oscAddress = message.AddressPattern();
        oscIn.oscAddress.replace("/", "").replace("(", "[").replace(")", "]");
        determineOscAction(oscIn);
    }

    void determineOscAction(oscResult& oscIn) {
        bool oscGetP = oscIn.oscAddress.startsWith("GetP#", Qt::CaseInsensitive);
        bool oscGetV = oscIn.oscAddress.startsWith("GetV#", Qt::CaseInsensitive);
        bool oscSet = !(oscGetP || oscGetV);

        int posDel = oscIn.oscAddress.indexOf("@", 0, Qt::CaseInsensitive);
        if (posDel > 0) {
            if (oscSet) {
                oscIn.oscGroup = oscIn.oscAddress.mid(0, posDel);
                oscIn.oscKey = oscIn.oscAddress.mid(posDel + 1, oscIn.oscAddress.length());
            } else {
                oscIn.oscGroup = oscIn.oscAddress.mid(5, posDel - 5);
                oscIn.oscKey = oscIn.oscAddress.mid(posDel + 1);
            }

            if (oscGetP) {
                doGetP(oscIn);
            } else if (oscGetV) {
                doGetV(oscIn);
            } else if (oscSet) {
                doSet(oscIn, oscIn.oscValue);
            }
        }
    }

    // OSC wants info from Mixxx -> Parameter
    void doGetP(oscResult& oscIn) {
        if (ControlObject::exists(ConfigKey(oscIn.oscGroup, oscIn.oscKey))) {
            auto proxy = std::make_unique<PollingControlProxy>(oscIn.oscGroup, oscIn.oscKey);
            OscFunctionsSendPtrType(m_pConfig,
                    oscIn.oscGroup,
                    oscIn.oscKey,
                    DefOscBodyType::FLOATBODY,
                    "",
                    0,
                    0,
                    static_cast<float>(proxy->get()));
            qDebug() << "OSC Msg Snd: Group, Key: Value:" << oscIn.oscGroup
                     << "," << oscIn.oscKey << ":" << proxy->get();
        }
    }

    // OSC wants info from Mixxx -> Value
    void doGetV(oscResult& oscIn) {
        if (ControlObject::exists(ConfigKey(oscIn.oscGroup, oscIn.oscKey))) {
            OscFunctionsSendPtrType(m_pConfig,
                    oscIn.oscGroup,
                    oscIn.oscKey,
                    DefOscBodyType::FLOATBODY,
                    "",
                    0,
                    0,
                    static_cast<float>(ControlObject::getControl(
                            oscIn.oscGroup, oscIn.oscKey)
                                               ->get()));
            qDebug() << "OSC Msg Rcvd: Get Group, Key: Value:" << oscIn.oscGroup
                     << "," << oscIn.oscKey << ":" << oscIn.oscValue;
        }
    }

    // Input from OSC -> Changes in Mixxx
    void doSet(oscResult& oscIn, float value) {
        if (ControlObject::exists(ConfigKey(oscIn.oscGroup, oscIn.oscKey))) {
            auto proxy = std::make_unique<PollingControlProxy>(oscIn.oscGroup, oscIn.oscKey);
            proxy->set(value);
            OscFunctionsSendPtrType(m_pConfig,
                    oscIn.oscGroup,
                    oscIn.oscKey,
                    DefOscBodyType::FLOATBODY,
                    "",
                    0,
                    0,
                    value);
            qDebug() << "OSC Msg Rcvd: Group, Key: Value:" << oscIn.oscGroup
                     << "," << oscIn.oscKey << ":" << value;
        } else {
            qDebug() << "OSC Msg Rcvd for NON-existing Control Object: Group, "
                        "Key: Value:"
                     << oscIn.oscGroup << "," << oscIn.oscKey << ":" << value;
        }
    }

    // trigger OSC to sync
    void sendOscSyncTriggers(UserSettingsPointer m_pConfig) {
        qDebug() << "Mixxx OSC SendSyncTrigger";
        int interval = m_pConfig
                               ->getValue(ConfigKey(
                                       "[OSC]", "OscSendSyncTriggersInterval"))
                               .toInt() /
                1000;
        int checkStamp = QDateTime::currentDateTime().toString("ss").toInt();

        qDebug() << "Mixxx OSC SENT SendSyncTrigger: checkStamp:" << checkStamp;
        if (checkStamp % interval == 0) {
            OscFunctionsSendPtrType(m_pConfig,
                    "[Osc]",
                    "OscSync",
                    DefOscBodyType::FLOATBODY,
                    "",
                    0,
                    0,
                    1);
            // qDebug() << "Mixxx OSC SENT SendSyncTrigger";
        }
    }
};

void RunOscReceiver(int OscPortIn, UserSettingsPointer m_pConfig) {
    qDebug() << "Mixxx OSC Service Thread started (RunOscReceiver -> OscReceivePacketListener)";
    OscReceivePacketListener listener(m_pConfig);
    UdpListeningReceiveSocket socket(IpEndpointName(IpEndpointName::ANY_ADDRESS, OscPortIn),
            &listener);
    socket.Run();
}

// #ifndef NO_OSC_TEST_MAIN

void OscReceiverMain(UserSettingsPointer m_pConfig) {
    if (m_pConfig->getValue<bool>(ConfigKey("[OSC]", "OscEnabled"))) {
        int CKOscPortInInt = m_pConfig->getValue(ConfigKey("[OSC]", "OscPortIn")).toInt();
        qDebug() << "OSC Enabled -> Started";

        // qDebuf print active receivers and ip's
        for (int i = 1; i <= 5; ++i) {
            QString receiverActive = QString("OscReceiver%1Active").arg(i);
            QString receiverIp = QString("OscReceiver%1Ip").arg(i);

            if (m_pConfig->getValue<bool>(ConfigKey("[OSC]", receiverActive))) {
                const QString& CKOscRecIp = m_pConfig->getValue(ConfigKey("[OSC]", receiverIp));
                qDebug() << QString(
                        "Mixxx OSC Receiver %1 with ip-address: %2 Activated")
                                    .arg(i)
                                    .arg(CKOscRecIp);
            } else {
                qDebug() << QString("Mixxx OSC Receiver %1 Not Activated").arg(i);
            }
        }

        for (int i = 1; i < 5; i++) {
            const QString& OscTrackGroup = QString("[Channel%1]").arg(i);
            OscNoTrackLoadedInGroup(m_pConfig, OscTrackGroup);
        }

        qDebug() << "Mixxx OSC Service Thread starting";
        std::thread oscThread(RunOscReceiver, CKOscPortInInt, m_pConfig);
        oscThread.detach();
        qDebug() << "Mixxx OSC Service Thread quit";
    } else {
        qDebug() << "Mixxx OSC Service NOT Enabled";
    }
}

// #endif /* NO_OSC_TEST_MAIN */
