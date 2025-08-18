#include "oscreceiver.h"

#include <stdio.h>

#include <QThread>
#include <iostream>

#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#endif
#include <signal.h>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "control/pollingcontrolproxy.h"
#include "moc_oscreceiver.cpp"
#include "oscfunctions.h"

std::atomic<bool> s_oscEnabled(false);
int s_ckOscPortOutInt = 0;
QList<std::pair<bool, QString>> s_receiverConfigs;
std::atomic<bool> s_configLoaded1stTimeFromFile(false);
static bool s_oscSendSyncTriggers(false);
static int s_oscSendSyncTriggersInterval;
// static int s_lastCheckStamp;
inline std::atomic<qint64> s_lastTriggerTime = 0;

namespace {
// const bool sDebug = true;
} // namespace

void oscFunctionsSendPtrType(
        const QString& oscGroup,
        const QString& oscKey,
        enum DefOscBodyType oscBodyType,
        const QString& oscMessageBodyQString,
        int oscMessageBodyInt,
        double oscMessageBodyDouble,
        float oscMessageBodyFloat);

OscReceiver::OscReceiver(
        UserSettingsPointer pConfig)
        : m_pConfig(pConfig),
          m_stopFlag(false) {};

OscReceiver::~OscReceiver() {
    stop();
}

static void errorCallback(int num, const char* msg, const char* where) {
    qWarning() << "[OSC] [OSCRECEIVER] -> Error" << num << "at" << where << ":" << msg;
}

static int quit_handler(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message data,
        void* user_data) {
    Q_UNUSED(path);
    Q_UNUSED(types);
    Q_UNUSED(argc);
    Q_UNUSED(argv);
    Q_UNUSED(data);
    Q_UNUSED(user_data);

    if (sDebug) {
        qDebug() << "[OSC] [OSCRECEIVER] -> Quitting";
    }
    return 0;
}

void OscReceiver::stop() {
    if (sDebug) {
        qDebug() << "[OSC] [OSCRECEIVER] -> Stop requested";
    }
    // Set a flag to stop the receiver loop
    m_stopFlag = true;

    if (QThread::currentThread()->isInterruptionRequested()) {
        if (sDebug) {
            qDebug() << "[OSC] [OSCRECEIVER] -> Thread requested to stop.";
        }
    }
    if (sDebug) {
        qDebug() << "[OSC] [OSCRECEIVER] -> OSC server stopped";
    }
}

static int messageCallback(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message data,
        void* user_data) {
    Q_UNUSED(types);
    Q_UNUSED(data);
    auto* worker = static_cast<OscReceiver*>(user_data);
    if (!worker) {
        qWarning() << "[OSC] [OSCRECEIVER] -> OSC message callback: Invalid user_data!";
        return 0;
    }
    if (argc < 1) {
        qWarning() << "[OSC] [OSCRECEIVER] -> Received OSC message on" << path
                   << "with no arguments";
        return 1;
    }

    OscResult oscIn;
    oscIn.oscAddress = QString::fromUtf8(path);
    // oscIn.oscAddressURL = QString("osc://localhost%1").arg(path);
    oscIn.oscAddressURL = QString("%1").arg(path);
    oscIn.oscValue = argv[0]->f; // Assuming first argument is a float

    if (sDebug) {
        qDebug() << "[OSC] [OSCRECEIVER] -> Received OSC message:"
                 << oscIn.oscAddress << "Value:" << oscIn.oscValue;
    }

    // oscIn.oscAddress.replace("/", "").replace("(", "[").replace(")", "]");
    if (sDebug) {
        // qDebug() << "[OSC] [OSCRECEIVER] -> Before translation " << oscIn.oscAddressURL;
    }

    if (s_oscSendSyncTriggers) {
        worker->sendOscSyncTriggers();
    }

    worker->determineOscAction(oscIn);
    return 0;
}

void OscReceiver::determineOscAction(OscResult& oscIn) {
    bool oscGetP = oscIn.oscAddressURL.startsWith("/Get/cop", Qt::CaseInsensitive);
    bool oscGetV = oscIn.oscAddressURL.startsWith("/Get/cov", Qt::CaseInsensitive);
    bool oscGetT = oscIn.oscAddressURL.startsWith("/Get/cot", Qt::CaseInsensitive);
    bool oscSet = !(oscGetP || oscGetV || oscGetT);

    oscIn.oscAddressURL.replace("/Get/cop", "").replace("/Get/cov", "").replace("/Get/cot", "");

    oscIn.oscAddress = translatePath(oscIn.oscAddressURL);
    if (sDebug) {
        qDebug() << "[OSC] [OSCRECEIVER] -> Before translation " << oscIn.oscAddressURL;
        qDebug() << "[OSC] [OSCRECEIVER] -> After translation " << oscIn.oscAddress;
        // qDebug() << "[OSC] [OSCRECEIVER] -> oscGetP " << oscGetP;
        // qDebug() << "[OSC] [OSCRECEIVER] -> oscGetV " << oscGetV;
        // qDebug() << "[OSC] [OSCRECEIVER] -> oscGetT " << oscGetT;
        // qDebug() << "[OSC] [OSCRECEIVER] -> oscSet " << oscSet;
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
void OscReceiver::doGetP(OscResult& oscIn) {
    if (sDebug) {
        qDebug() << "[OSC] [OSCRECEIVER] -> doGetP triggered oscIn.oscGroup" << oscIn.oscGroup
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
void OscReceiver::doGetV(OscResult& oscIn) {
    if (sDebug) {
        qDebug() << "[OSC] [OSCRECEIVER] -> doGetV triggered oscIn.oscGroup" << oscIn.oscGroup
                 << " oscIn.oscKey " << oscIn.oscKey;
    }
    if (ControlObject::exists(ConfigKey(oscIn.oscGroup, oscIn.oscKey))) {
        auto proxy = std::make_unique<PollingControlProxy>(oscIn.oscGroup, oscIn.oscKey);
        // for future use when prefix /cop is introduced in osc-messages
        // oscIn.oscGroup = QString("%1%2").arg("/cov", oscIn.oscGroup);
        oscFunctionsSendPtrType(
                oscIn.oscGroup,
                oscIn.oscKey,
                DefOscBodyType::FLOATBODY,
                "",
                0,
                0,
                static_cast<float>(proxy->get()));
        if (sDebug) {
            qDebug() << "[OSC] [OSCRECEIVER] -> Msg Rcvd: Get Group, Key: Value:" << oscIn.oscGroup
                     << "," << oscIn.oscKey << ":" << oscIn.oscValue;
        }
    }
}

void OscReceiver::doGetT(OscResult& oscIn) {
    if (sDebug) {
        qDebug() << "[OSC] [OSCRECEIVER] -> doGetT triggered oscIn.oscGroup" << oscIn.oscGroup
                 << " oscIn.oscKey " << oscIn.oscKey;
    }

    // QString searchOscKey = QString(oscIn.oscGroup + oscIn.oscKey);
    if (sDebug) {
        qDebug() << "[OSC] [OSCRECEIVER] -> Msg Rcvd: Get Group, TrackInfo: " << oscIn.oscGroup
                 << "," << oscIn.oscKey;
    }

    QString sendOscValue = getTrackInfo(oscIn.oscGroup, oscIn.oscKey);

    // Send the OSC message using the stored value
    oscFunctionsSendPtrType(
            oscIn.oscGroup,
            oscIn.oscKey,
            DefOscBodyType::STRINGBODY,
            escapeStringToJsonUnicode(sendOscValue),
            0,
            0,
            0);

    if (sDebug) {
        qDebug() << "[OSC] [OSCRECEIVER] -> Msg Rcvd: Get TrackInfo, Key: Value:" << oscIn.oscGroup
                 << "," << oscIn.oscKey << ":" << sendOscValue;
    }
}

// Input from OSC -> Changes in Mixxx
void OscReceiver::doSet(OscResult& oscIn, float value) {
    if (sDebug) {
        qDebug() << "[OSC] [OSCRECEIVER] -> doSet triggered oscIn.oscGroup" << oscIn.oscGroup
                 << " oscIn.oscKey " << oscIn.oscKey;
    }
    if (ControlObject::exists(ConfigKey(oscIn.oscGroup, oscIn.oscKey))) {
        auto proxy = std::make_unique<PollingControlProxy>(oscIn.oscGroup, oscIn.oscKey);
        // proxy->set(value);
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
            qDebug() << "[OSC] [OSCRECEIVER] -> Msg Rcvd: Group, Key: Value:" << oscIn.oscGroup
                     << "," << oscIn.oscKey << ":" << value;
        }
    } else {
        if (sDebug) {
            qDebug() << "[OSC] [OSCRECEIVER] -> Msg Rcvd for non-existing Control Object: Group, "
                        "Key: Value:"
                     << oscIn.oscGroup << "," << oscIn.oscKey << ":" << value;
        }
    }
}

// trigger OSC to sync
void OscReceiver::sendOscSyncTriggers() {
    if (sDebug) {
        qDebug() << "[OSC] [OSCRECEIVER] -> Mixxx OSC SendSyncTrigger";
    }

    // Get current timestamp in milliseconds
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

    // Check if enough time has passed since the last trigger, at least interval
    if (currentTime - s_lastTriggerTime >= s_oscSendSyncTriggersInterval) {
        // Execute the OSC send function
        oscFunctionsSendPtrType(
                "[Osc]",
                "OscSync",
                DefOscBodyType::FLOATBODY,
                "",
                0,
                0,
                1);

        // Update the last trigger timestamp
        s_lastTriggerTime = currentTime;

        if (sDebug) {
            qDebug() << "[OSC] [OSCRECEIVER] -> Mixxx OSC SENT SendSyncTrigger at" << currentTime;
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

// liblo in own thread
int OscReceiver::startOscReceiver(int oscPortin) {
    m_oscPortIn = oscPortin;
    std::string portStr = std::to_string(oscPortin);
    lo_server_thread st = lo_server_thread_new_with_proto(portStr.c_str(), LO_UDP, errorCallback);
    lo_server s = lo_server_thread_get_server(st);
    lo_server_thread_add_method(st, "/quit", "", quit_handler, NULL);
    lo_server_thread_add_method(st, NULL, NULL, messageCallback, s);
    lo_server_thread_start(st);
    if (sDebug) {
        qDebug() << "[OSC] [OSCRECEIVER] -> Receiver started and awaiting messages...";
    }

    while (!m_stopFlag) {
        QThread::msleep(100); // Sleep for a short period
    }

    // Stop the server when done
    lo_server_thread_stop(st);
    lo_server_thread_free(st);
    if (sDebug) {
        qDebug() << "[OSC] [OSCRECEIVER] -> Receiver stopped";
    }
    return 0;
}

void OscReceiver::checkResponsiveness() {
    // Check if the last response was more than 5 seconds ago
    if (m_lastResponseTime.secsTo(QDateTime::currentDateTime()) > 5) {
        qWarning() << "[OSC] [OSCRECEIVER] -> OSC server is unresponsive. Restarting...";
        restartOscReceiver(m_oscPortIn); // Restart the OSC server
    }
}

void OscReceiver::restartOscReceiver(int oscPortin) {
    // Stop the current OSC server
    stop();

    // Wait for the server to stop
    QThread::msleep(100);

    // Reset the stop flag
    m_stopFlag = false;

    // Restart the OSC server
    startOscReceiver(oscPortin);
}

// void OscReceiver::oscReceiverMain(UserSettingsPointer pConfig) {
//     if (pConfig->getValue<bool>(ConfigKey("[OSC]", "OscEnabled"))) {
//         if (sDebug) {
//             qDebug() << "[OSC] [OSCRECEIVER] -> Enabled -> Started";
//         }
//         loadOscConfiguration(pConfig);
//         for (const auto& receiver : s_receiverConfigs) {
//             int i = 1;
//             if (receiver.first) { // Check if the receiver is active
//                 QByteArray receiverIpBa = receiver.second.toLocal8Bit();
//                 if (sDebug) {
//                     qDebug() << QString(
//                             "[OSC] [OSCRECEIVER] -> Mixxx OSC Receiver %1 with "
//                             "ip-address: %2 Activated")
//                                         .arg(i)
//                                         .arg(receiverIpBa);
//                 }
//             } else {
//                 if (sDebug) {
//                     qDebug()
//                             << QString("[OSC] [OSCRECEIVER] -> Mixxx OSC "
//                                        "Receiver %1 Not Activated")
//                                        .arg(i);
//                 }
//             }
//             i = i++;
//         }
//     } else {
//         if (sDebug) {
//             qDebug() << "[OSC] [OSCRECEIVER] -> Mixxx OSC Service NOT Enabled";
//         }
//     }
// }

void OscReceiver::loadOscConfiguration(UserSettingsPointer pConfig) {
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
        s_oscSendSyncTriggers = m_pConfig->getValue<bool>(
                ConfigKey("[OSC]", "OscSendSyncTriggers"));

        s_oscSendSyncTriggersInterval = m_pConfig
                                                ->getValue(ConfigKey(
                                                        "[OSC]", "OscSendSyncTriggersInterval"))
                                                .toInt();

        s_ckOscPortOutInt = pConfig->getValue(ConfigKey("[OSC]", "OscPortOut")).toInt();

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
