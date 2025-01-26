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

namespace {
const bool sDebug = true;
}

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
            qDebug() << "[OSC] Error parsing Msg from "
                     << oscMessage.AddressPattern() << " error: " << e.what();
        }
    }

    void processOscMessage(const osc::ReceivedMessage& message) {
        OscResult oscIn;
        osc::ReceivedMessageArgumentStream args = message.ArgumentStream();
        args >> oscIn.oscValue >> osc::EndMessage;
        oscIn.oscAddressURL = message.AddressPattern();
        // oscIn.oscAddress.replace("/", "").replace("(", "[").replace(")", "]");
        if (sDebug) {
            qDebug() << "[OSC] OSCRECEIVER Before translation " << oscIn.oscAddressURL;
        }
        determineOscAction(oscIn);
    }

    void determineOscAction(OscResult& oscIn) {
        //        bool oscGetP = oscIn.oscAddress.startsWith("GetP#", Qt::CaseInsensitive);
        //        bool oscGetV = oscIn.oscAddress.startsWith("GetV#", Qt::CaseInsensitive);
        //        bool oscGetT = oscIn.oscAddress.startsWith("GetT#", Qt::CaseInsensitive);

        bool oscGetP = oscIn.oscAddressURL.startsWith("/Get/cop", Qt::CaseInsensitive);
        bool oscGetV = oscIn.oscAddressURL.startsWith("/Get/cov", Qt::CaseInsensitive);
        bool oscGetT = oscIn.oscAddressURL.startsWith("/Get/cot", Qt::CaseInsensitive);
        bool oscSet = !(oscGetP || oscGetV || oscGetT);

        oscIn.oscAddressURL.replace("/Get/cop", "").replace("/Get/cov", "").replace("/Get/cot", "");

        oscIn.oscAddress = translatePath(oscIn.oscAddressURL);
        if (sDebug) {
            qDebug() << "[OSC] OSCRECEIVER After translation " << oscIn.oscAddress;
            qDebug() << "[OSC] OSCRECEIVER Before translation " << oscIn.oscAddressURL;
            qDebug() << "[OSC] OSCRECEIVER oscGetP " << oscGetP;
            qDebug() << "[OSC] OSCRECEIVER oscGetV " << oscGetV;
            qDebug() << "[OSC] OSCRECEIVER oscGetT " << oscGetT;
            qDebug() << "[OSC] OSCRECEIVER oscSet " << oscSet;
        }
        // int posDel = oscIn.oscAddress.indexOf("@", 0, Qt::CaseInsensitive);
        int posDel = oscIn.oscAddress.indexOf(",", 0, Qt::CaseInsensitive);
        if (posDel > 0) {
            if (oscSet) {
                // translatePath(oscIn.oscAddress);
                oscIn.oscGroup = oscIn.oscAddress.mid(0, posDel);
                oscIn.oscKey = oscIn.oscAddress.mid(posDel + 1, oscIn.oscAddress.length());

            } else {
                // oscIn.oscGroup = oscIn.oscAddress.mid(5, posDel - 5);
                // oscIn.oscKey = oscIn.oscAddress.mid(posDel + 1);

                oscIn.oscGroup = oscIn.oscAddress.mid(0, posDel);
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
        if (sDebug) {
            qDebug() << "[OSC] doGetP triggered oscIn.oscGroup" << oscIn.oscGroup
                     << " oscIn.oscKey " << oscIn.oscKey;
        }
        if (ControlObject::exists(ConfigKey(oscIn.oscGroup, oscIn.oscKey))) {
            auto proxy = std::make_unique<PollingControlProxy>(oscIn.oscGroup, oscIn.oscKey);
            // for future use when prefix /cop is introduced in osc-messages
            // oscIn.oscGroup = QString("%1%2").arg("/cop", oscIn.oscGroup);
            oscFunctionsSendPtrType(pConfig,
                    oscIn.oscGroup,
                    oscIn.oscKey,
                    DefOscBodyType::FLOATBODY,
                    "",
                    0,
                    0,
                    static_cast<float>(proxy->get()));
            if (sDebug) {
                qDebug() << "[OSC] Msg Snd: Group, Key: Value:" << oscIn.oscGroup
                         << "," << oscIn.oscKey << ":" << proxy->get();
            }
        }
    }

    // OSC wants info from Mixxx -> Value
    void doGetV(OscResult& oscIn) {
        if (sDebug) {
            qDebug() << "[OSC] doGetV triggered oscIn.oscGroup" << oscIn.oscGroup
                     << " oscIn.oscKey " << oscIn.oscKey;
        }
        if (ControlObject::exists(ConfigKey(oscIn.oscGroup, oscIn.oscKey))) {
            // for future use when prefix /cop is introduced in osc-messages
            // oscIn.oscGroup = QString("%1%2").arg("/cov", oscIn.oscGroup);
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
            if (sDebug) {
                qDebug() << "[OSC] Msg Rcvd: Get Group, Key: Value:" << oscIn.oscGroup
                         << "," << oscIn.oscKey << ":" << oscIn.oscValue;
            }
        }
    }

    // OSC wants info from Mixxx -> TrackArtist & TrackTitle
    void doGetT(OscResult& oscIn) {
        if (sDebug) {
            qDebug() << "[OSC] doGetT triggered oscIn.oscGroup" << oscIn.oscGroup
                     << " oscIn.oscKey " << oscIn.oscKey;
        }
        QString searchOscKey = QString(oscIn.oscGroup + oscIn.oscKey);
        if (sDebug) {
            qDebug() << "[OSC] Msg Rcvd: Get Group, TrackInfo: " << oscIn.oscGroup
                     << "," << oscIn.oscKey;
        }
        const QString& sendOscValue = pConfig->getValue(ConfigKey("[OSC]", searchOscKey));
        // for future use when prefix /cop is introduced in osc-messages
        // oscIn.oscGroup = QString("%1%2").arg("/cot", oscIn.oscGroup);
        oscFunctionsSendPtrType(pConfig,
                oscIn.oscGroup,
                oscIn.oscKey,
                DefOscBodyType::STRINGBODY,
                escapeStringToJsonUnicode(sendOscValue),
                0,
                0,
                0);
        if (sDebug) {
            qDebug() << "[OSC] Msg Rcvd: Get TrackInfo, Key: Value:" << oscIn.oscGroup
                     << "," << oscIn.oscKey << ":" << sendOscValue;
        }
    }

    // Input from OSC -> Changes in Mixxx
    void doSet(OscResult& oscIn, float value) {
        if (sDebug) {
            qDebug() << "[OSC] doSet triggered oscIn.oscGroup" << oscIn.oscGroup
                     << " oscIn.oscKey " << oscIn.oscKey;
        }
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
            if (sDebug) {
                qDebug() << "[OSC] Msg Rcvd: Group, Key: Value:" << oscIn.oscGroup
                         << "," << oscIn.oscKey << ":" << value;
            }
        } else {
            if (sDebug) {
                qDebug() << "[OSC] Msg Rcvd for non-existing Control Object: Group, "
                            "Key: Value:"
                         << oscIn.oscGroup << "," << oscIn.oscKey << ":" << value;
            }
        }
    }

    // trigger OSC to sync
    void sendOscSyncTriggers(UserSettingsPointer pConfig) {
        if (sDebug) {
            qDebug() << "[OSC] Mixxx OSC SendSyncTrigger";
        }
        int interval = pConfig
                               ->getValue(ConfigKey(
                                       "[OSC]", "OscSendSyncTriggersInterval"))
                               .toInt() /
                1000;
        int checkStamp = QDateTime::currentDateTime().toString("ss").toInt();
        if (sDebug) {
            qDebug() << "[OSC] Mixxx OSC SENT SendSyncTrigger: checkStamp:" << checkStamp;
        }
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
    if (sDebug) {
        qDebug() << "[OSC] Mixxx OSC Service Thread started (RunOscReceiver -> "
                    "OscReceivePacketListener)";
    }
    OscReceivePacketListener listener(pConfig);
    UdpListeningReceiveSocket socket(IpEndpointName(IpEndpointName::ANY_ADDRESS, oscPortIn),
            &listener);
    socket.Run();
}

// #ifndef NO_OSC_TEST_MAIN

void oscReceiverMain(UserSettingsPointer pConfig) {
    if (pConfig->getValue<bool>(ConfigKey("[OSC]", "OscEnabled"))) {
        int ckOscPortInInt = pConfig->getValue(ConfigKey("[OSC]", "OscPortIn")).toInt();
        if (sDebug) {
            qDebug() << "[OSC] Enabled -> Started";
        }

        // qDebuf print active receivers and ip's
        for (int i = 1; i <= 5; ++i) {
            QString receiverActive = QString("OscReceiver%1Active").arg(i);
            QString receiverIp = QString("OscReceiver%1Ip").arg(i);

            if (pConfig->getValue<bool>(ConfigKey("[OSC]", receiverActive))) {
                const QString& ckOscRecIp = pConfig->getValue(ConfigKey("[OSC]", receiverIp));
                if (sDebug) {
                    qDebug() << QString(
                            "[OSC] Mixxx OSC Receiver %1 with ip-address: %2 Activated")
                                        .arg(i)
                                        .arg(ckOscRecIp);
                }
            } else {
                if (sDebug) {
                    qDebug() << QString("[OSC] Mixxx OSC Receiver %1 Not Activated").arg(i);
                }
            }
        }

        for (int i = 1; i < 5; i++) {
            const QString& oscTrackGroup = QString("[Channel%1]").arg(i);
            sendNoTrackLoadedToOscClients(pConfig, oscTrackGroup);
        }
        if (sDebug) {
            qDebug() << "[OSC] Mixxx OSC Service Thread starting";
        }
        std::thread oscThread(runOscReceiver, ckOscPortInInt, pConfig);
        oscThread.detach();
        if (sDebug) {
            qDebug() << "[OSC] Mixxx OSC Service Thread quit";
        }
    } else {
        if (sDebug) {
            qDebug() << "[OSC] Mixxx OSC Service NOT Enabled";
        }
    }
}

// #endif /* NO_OSC_TEST_MAIN */
