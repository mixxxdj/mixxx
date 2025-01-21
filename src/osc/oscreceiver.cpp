// Just a comment to het something changed in the pr so the mac os 12
// // compiler would retry compiling after the brownout
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

void oscFunctionsSendPtrType(UserSettingsPointer pConfig,
        const QString& oscGroup,
        const QString& oscKey,
        enum DefOscBodyType oscBodyType,
        const QString& oscMessageBodyQString,
        int oscMessageBodyInt,
        double oscMessageBodyDouble,
        float oscMessageBodyFloat);

class OscReceivePacketListener : public osc::OscPacketListener {
  public:
    UserSettingsPointer pConfig;
    OscReceivePacketListener(UserSettingsPointer aPointerHerePlease) {
        pConfig = aPointerHerePlease;
    };

  private:
    void ProcessMessage(const osc::ReceivedMessage& oscMessage,
            const IpEndpointName& remoteEndpoint) {
        (void)remoteEndpoint;
        try {
            if (pConfig->getValue<bool>(ConfigKey("[OSC]", "OscSendSyncTriggers"))) {
                sendOscSyncTriggers(pConfig);
            }
            processOscMessage(oscMessage);
        } catch (const osc::Exception& e) {
            qDebug() << "OSC Error parsing Msg from "
                     << oscMessage.AddressPattern() << " error: " << e.what();
        }
    }

    void processOscMessage(const osc::ReceivedMessage& message) {
        OscResult oscIn;
        osc::ReceivedMessageArgumentStream args = message.ArgumentStream();
        args >> oscIn.oscValue >> osc::EndMessage;
        oscIn.oscAddress = message.AddressPattern();
        oscIn.oscAddress.replace("/", "").replace("(", "[").replace(")", "]");
        determineOscAction(oscIn);
    }

    void determineOscAction(OscResult& oscIn) {
        bool oscGetP = oscIn.oscAddress.startsWith("GetP#", Qt::CaseInsensitive);
        bool oscGetV = oscIn.oscAddress.startsWith("GetV#", Qt::CaseInsensitive);
        bool oscGetT = oscIn.oscAddress.startsWith("GetT#", Qt::CaseInsensitive);
        bool oscSet = !(oscGetP || oscGetV || oscGetT);

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
            } else if (oscGetT) {
                doGetT(oscIn);
            } else if (oscSet) {
                doSet(oscIn, oscIn.oscValue);
            }
        }
    }

    // OSC wants info from Mixxx -> Parameter
    void doGetP(OscResult& oscIn) {
        if (ControlObject::exists(ConfigKey(oscIn.oscGroup, oscIn.oscKey))) {
            auto proxy = std::make_unique<PollingControlProxy>(oscIn.oscGroup, oscIn.oscKey);
            oscFunctionsSendPtrType(pConfig,
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
    void doGetV(OscResult& oscIn) {
        if (ControlObject::exists(ConfigKey(oscIn.oscGroup, oscIn.oscKey))) {
            oscFunctionsSendPtrType(pConfig,
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

    // OSC wants info from Mixxx -> TrackArtist & TrackTitle
    void doGetT(OscResult& oscIn) {
        QString searchOscKey = QString(oscIn.oscGroup + oscIn.oscKey);
        qDebug() << "OSC Msg Rcvd: Get Group, TrackInfo: " << oscIn.oscGroup
                 << "," << oscIn.oscKey;
        const QString& sendOscValue = pConfig->getValue(ConfigKey("[OSC]", searchOscKey));
        oscFunctionsSendPtrType(pConfig,
                oscIn.oscGroup,
                oscIn.oscKey,
                DefOscBodyType::STRINGBODY,
                escapeStringToJsonUnicode(sendOscValue),
                0,
                0,
                0);

        qDebug() << "OSC Msg Rcvd: Get TrackInfo, Key: Value:" << oscIn.oscGroup
                 << "," << oscIn.oscKey << ":" << sendOscValue;
    }

    // Input from OSC -> Changes in Mixxx
    void doSet(OscResult& oscIn, float value) {
        if (ControlObject::exists(ConfigKey(oscIn.oscGroup, oscIn.oscKey))) {
            auto proxy = std::make_unique<PollingControlProxy>(oscIn.oscGroup, oscIn.oscKey);
            proxy->set(value);
            oscFunctionsSendPtrType(pConfig,
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
            qDebug() << "OSC Msg Rcvd for non-existing Control Object: Group, "
                        "Key: Value:"
                     << oscIn.oscGroup << "," << oscIn.oscKey << ":" << value;
        }
    }

    // trigger OSC to sync
    void sendOscSyncTriggers(UserSettingsPointer pConfig) {
        qDebug() << "Mixxx OSC SendSyncTrigger";
        int interval = pConfig
                               ->getValue(ConfigKey(
                                       "[OSC]", "OscSendSyncTriggersInterval"))
                               .toInt() /
                1000;
        int checkStamp = QDateTime::currentDateTime().toString("ss").toInt();

        qDebug() << "Mixxx OSC SENT SendSyncTrigger: checkStamp:" << checkStamp;
        if (checkStamp % interval == 0) {
            oscFunctionsSendPtrType(pConfig,
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

void runOscReceiver(int oscPortIn, UserSettingsPointer pConfig) {
    qDebug() << "Mixxx OSC Service Thread started (RunOscReceiver -> OscReceivePacketListener)";
    OscReceivePacketListener listener(pConfig);
    UdpListeningReceiveSocket socket(IpEndpointName(IpEndpointName::ANY_ADDRESS, oscPortIn),
            &listener);
    socket.Run();
}

// #ifndef NO_OSC_TEST_MAIN

void oscReceiverMain(UserSettingsPointer pConfig) {
    if (pConfig->getValue<bool>(ConfigKey("[OSC]", "OscEnabled"))) {
        int ckOscPortInInt = pConfig->getValue(ConfigKey("[OSC]", "OscPortIn")).toInt();
        qDebug() << "OSC Enabled -> Started";

        // qDebuf print active receivers and ip's
        for (int i = 1; i <= 5; ++i) {
            QString receiverActive = QString("OscReceiver%1Active").arg(i);
            QString receiverIp = QString("OscReceiver%1Ip").arg(i);

            if (pConfig->getValue<bool>(ConfigKey("[OSC]", receiverActive))) {
                const QString& ckOscRecIp = pConfig->getValue(ConfigKey("[OSC]", receiverIp));
                qDebug() << QString(
                        "Mixxx OSC Receiver %1 with ip-address: %2 Activated")
                                    .arg(i)
                                    .arg(ckOscRecIp);
            } else {
                qDebug() << QString("Mixxx OSC Receiver %1 Not Activated").arg(i);
            }
        }

        for (int i = 1; i < 5; i++) {
            const QString& oscTrackGroup = QString("[Channel%1]").arg(i);
            sendNoTrackLoadedToOscClients(pConfig, oscTrackGroup);
        }

        qDebug() << "Mixxx OSC Service Thread starting";
        std::thread oscThread(runOscReceiver, ckOscPortInInt, pConfig);
        oscThread.detach();
        qDebug() << "Mixxx OSC Service Thread quit";
    } else {
        qDebug() << "Mixxx OSC Service NOT Enabled";
    }
}

// #endif /* NO_OSC_TEST_MAIN */
