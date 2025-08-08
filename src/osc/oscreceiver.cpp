//// Just a comment to get something changed in the pr so the CI restarts 20250730

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

std::atomic<bool> s_oscEnabled(false);
int s_ckOscPortOutInt = 0;
std::atomic<int> s_ckOscPortInInt = 0;
QList<std::pair<bool, QString>> s_receiverConfigs;
std::atomic<bool> s_configLoaded1stTimeFromFile(false);
static bool s_oscSendSyncTriggers(false);
static int s_oscSendSyncTriggersInterval;
std::atomic<qint64> s_lastTriggerTime = 0;

namespace {
const bool sDebug = true;
}

void oscFunctionsSendPtrType(
        const QString& oscGroup,
        const QString& oscKey,
        enum DefOscBodyType oscBodyType,
        const QString& oscMessageBodyQString,
        int oscMessageBodyInt,
        double oscMessageBodyDouble,
        float oscMessageBodyFloat);

void sendNoTrackLoadedToOscClients(
        const QString& oscGroup);

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
            if (s_oscSendSyncTriggers) {
                sendOscSyncTriggers();
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
            oscFunctionsSendPtrType(
                    oscIn.oscGroup,
                    oscIn.oscKey,
                    DefOscBodyType::FLOATBODY,
                    "",
                    0,
                    0,
                    static_cast<float>(proxy->getParameter()));
            if (sDebug) {
                qDebug() << "[OSC] [OSCRECEIVER] -> Msg Snd: Group, Key: Value:" << oscIn.oscGroup
                         << "," << oscIn.oscKey << ":" << proxy->getParameter();
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
            oscFunctionsSendPtrType(
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
        if (sDebug) {
            qDebug() << "[OSC] Msg Rcvd: Get Group, TrackInfo: " << oscIn.oscGroup
                     << "," << oscIn.oscKey;
        }
        QString sendOscValue = getTrackInfo(oscIn.oscGroup, oscIn.oscKey);
        // for future use when prefix /cop is introduced in osc-messages
        // oscIn.oscGroup = QString("%1%2").arg("/cot", oscIn.oscGroup);
        oscFunctionsSendPtrType(
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
            proxy->setParameter(value);
            oscFunctionsSendPtrType(
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
    void sendOscSyncTriggers() {
        if (sDebug) {
            qDebug() << "[OSC] [OSCRECEIVER] -> Mixxx OSC SendSyncTrigger";
        }

        // Get current timestamp in milliseconds
        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

        // Check if enough time has passed since the last trigger, at least interval
        if (currentTime - s_lastTriggerTime >= s_oscSendSyncTriggersInterval) {
            oscFunctionsSendPtrType(
                    "[Osc]",
                    "OscSync",
                    DefOscBodyType::FLOATBODY,
                    "",
                    0,
                    0,
                    1);

            s_lastTriggerTime = currentTime;

            if (sDebug) {
                qDebug() << "[OSC] [OSCRECEIVER] -> Mixxx OSC SENT "
                            "SendSyncTrigger at"
                         << currentTime;
            }
        } else {
            if (sDebug) {
                qDebug() << "[OSC] [OSCRECEIVER] -> Mixxx OSC NO SendSyncTrigger SENT. "
                         << "Last trigger:" << s_lastTriggerTime
                         << " | Current time:" << currentTime
                         << " | Interval required:" << s_oscSendSyncTriggersInterval
                         << " | Time since last trigger:" << (currentTime - s_lastTriggerTime);
            }
        }
    }
};

void runOscReceiver(UserSettingsPointer pConfig) {
    if (sDebug) {
        qDebug() << "[OSC] Mixxx OSC Service Thread started (RunOscReceiver -> "
                    "OscReceivePacketListener)";
    }
    OscReceivePacketListener listener(pConfig);
    UdpListeningReceiveSocket socket(IpEndpointName(IpEndpointName::ANY_ADDRESS, s_ckOscPortInInt),
            &listener);
    socket.Run();
}

// #ifndef NO_OSC_TEST_MAIN
void oscReceiverMain(UserSettingsPointer pConfig) {
    if (pConfig->getValue<bool>(ConfigKey("[OSC]", "OscEnabled"))) {
        loadOscConfiguration(pConfig);
        s_ckOscPortInInt = pConfig->getValue(ConfigKey("[OSC]", "OscPortIn")).toInt();
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
            sendNoTrackLoadedToOscClients(oscTrackGroup);
        }
        if (sDebug) {
            qDebug() << "[OSC] Mixxx OSC Service Thread starting";
        }
        std::thread oscThread(runOscReceiver, pConfig);
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

void loadOscConfiguration(UserSettingsPointer pConfig) {
    if (!s_configLoaded1stTimeFromFile.load()) {
        QMutexLocker locker(&s_configMutex);
        if (sDebug) {
            qDebug() << "[OSC] [OSCRECEIVER] -> loadOscConfiguration -> start loading config";
        }
        if (!pConfig) {
            qWarning() << "[OSC] [OSCRECEIVER] -> pConfig is nullptr! Aborting OSC send.";
            return;
        }
        if (!pConfig) {
            qWarning() << "[OSC] [OSCRECEIVER] -> pConfig is nullptr! Aborting reload.";
            return;
        }
        if (pConfig->getValue<bool>(ConfigKey("[OSC]", "OscEnabled"))) {
            s_oscEnabled.store(true);
            if (sDebug) {
                qDebug() << "[OSC] [OSCRECEIVER] -> loadOscConfiguration -> "
                            "s_oscEnabled set to TRUE";
            }
        }
        if (pConfig->getValue<bool>(ConfigKey("[OSC]", "OscEnabled"))) {
            s_oscEnabled = true;
        }
        s_oscSendSyncTriggers = pConfig->getValue<bool>(
                ConfigKey("[OSC]", "OscSendSyncTriggers"));

        s_oscSendSyncTriggersInterval = pConfig
                                                ->getValue(ConfigKey(
                                                        "[OSC]", "OscSendSyncTriggersInterval"))
                                                .toInt();

        s_ckOscPortOutInt = pConfig->getValue(ConfigKey("[OSC]", "OscPortOut")).toInt();
        s_ckOscPortInInt = pConfig->getValue(ConfigKey("[OSC]", "OscPortIn")).toInt();

        // Clear existing receiver configurations
        s_receiverConfigs.clear();

        // List of receiver configurations
        const QList<std::pair<QString, QString>> receivers = {
                {"[OSC]", "OscReceiver1"},
                {"[OSC]", "OscReceiver2"},
                {"[OSC]", "OscReceiver3"},
                {"[OSC]", "OscReceiver4"},
                {"[OSC]", "OscReceiver5"}};

        // Store receiver configurations
        for (const auto& receiver : receivers) {
            bool active = pConfig->getValue<bool>(
                    ConfigKey(receiver.first, receiver.second + "Active"));
            QString ip = pConfig->getValue(ConfigKey(receiver.first, receiver.second + "Ip"));
            s_receiverConfigs.append({active, ip});
        }
        // Mark configuration as initialized
        s_configLoaded1stTimeFromFile.store(true);
        sendNoTrackLoadedToOscClients("[Channel1]");
        sendNoTrackLoadedToOscClients("[Channel2]");
        sendNoTrackLoadedToOscClients("[Channel3]");
        sendNoTrackLoadedToOscClients("[Channel4]");
        if (sDebug) {
            qDebug() << "[OSC] [OSCRECEIVER] -> OSC configuration loaded.";
        }
    }
}

// #endif /* NO_OSC_TEST_MAIN */
