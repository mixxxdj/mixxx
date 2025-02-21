#include "oscreceiver.h"

#include <QThread>
// #include <QDebug>
// #include <cstdlib>
// #include <cstring>
#include <stdio.h>

#include <iostream>

#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#endif
#include <signal.h>
// #include <lo/lo.h>
//  #include <lo/lo_cpp.h>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "control/pollingcontrolproxy.h"
#include "moc_oscreceiver.cpp"
#include "oscfunctions.h"

namespace {
// const bool sDebug = true;
} // namespace

void oscFunctionsSendPtrType(UserSettingsPointer pConfig,
        const QString& oscGroup,
        const QString& oscKey,
        enum DefOscBodyType oscBodyType,
        const QString& oscMessageBodyQString,
        int oscMessageBodyInt,
        double oscMessageBodyDouble,
        float oscMessageBodyFloat);

OscReceiver::OscReceiver(
        UserSettingsPointer pConfig,
        QObject* parent)
        : m_pConfig(pConfig),
          m_stopFlag(false) {};

OscReceiver::~OscReceiver() {
    stop();
}

static void errorCallback(int num, const char* msg, const char* where) {
    qWarning() << "[OSC] [OSCREVEIVER] -> Error" << num << "at" << where << ":" << msg;
}

static int quit_handler(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message data,
        void* user_data) {
    qDebug() << "[OSC] [OSCREVEIVER] -> Quitting";
    return 0;
}

void OscReceiver::stop() {
    qDebug() << "[OSC] [OSCREVEIVER] -> Stop requested";
    // Set a flag to stop the receiver loop
    m_stopFlag = true;

    if (QThread::currentThread()->isInterruptionRequested()) {
        qDebug() << "[OSC] [OSCREVEIVER] -> Thread requested to stop.";
    }
    qDebug() << "[OSC] [OSCREVEIVER] -> OSC server stopped";
}

static int messageCallback(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message data,
        void* user_data) {
    Q_UNUSED(types);

    auto* worker = static_cast<OscReceiver*>(user_data);
    if (!worker) {
        qWarning() << "[OSC] [OSCREVEIVER] -> OSC message callback: Invalid user_data!";
        return 0;
    }
    if (argc < 1) {
        qWarning() << "[OSC] [OSCREVEIVER] -> Received OSC message on" << path
                   << "with no arguments";
        return 1;
    }

    OscResult oscIn;
    oscIn.oscAddress = QString::fromUtf8(path);
    // oscIn.oscAddressURL = QString("osc://localhost%1").arg(path);
    oscIn.oscAddressURL = QString("%1").arg(path);
    oscIn.oscValue = argv[0]->f; // Assuming first argument is a float

    qDebug() << "[OSC] [OSCREVEIVER] -> Received OSC message:"
             << oscIn.oscAddress << "Value:" << oscIn.oscValue;
    // oscIn.oscAddress.replace("/", "").replace("(", "[").replace(")", "]");
    if (sDebug) {
        // qDebug() << "[OSC] [OSCREVEIVER] -> Before translation " << oscIn.oscAddressURL;
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
        qDebug() << "[OSC] [OSCREVEIVER] -> Before translation " << oscIn.oscAddressURL;
        qDebug() << "[OSC] [OSCREVEIVER] -> After translation " << oscIn.oscAddress;
        qDebug() << "[OSC] [OSCREVEIVER] -> oscGetP " << oscGetP;
        qDebug() << "[OSC] [OSCREVEIVER] -> oscGetV " << oscGetV;
        qDebug() << "[OSC] [OSCREVEIVER] -> oscGetT " << oscGetT;
        qDebug() << "[OSC] [OSCREVEIVER] -> oscSet " << oscSet;
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
        qDebug() << "[OSC] [OSCREVEIVER] -> doGetP triggered oscIn.oscGroup" << oscIn.oscGroup
                 << " oscIn.oscKey " << oscIn.oscKey;
    }
    if (ControlObject::exists(ConfigKey(oscIn.oscGroup, oscIn.oscKey))) {
        auto proxy = std::make_unique<PollingControlProxy>(oscIn.oscGroup, oscIn.oscKey);
        // for future use when prefix /cop is introduced in osc-messages
        // oscIn.oscGroup = QString("%1%2").arg("/cop", oscIn.oscGroup);
        oscFunctionsSendPtrType(m_pConfig,
                oscIn.oscGroup,
                oscIn.oscKey,
                DefOscBodyType::FLOATBODY,
                // DefOscBodyType::DOUBLEBODY,
                "",
                0,
                0,
                static_cast<float>(proxy->getParameter()));
        // proxy->getParameter());
        if (sDebug) {
            qDebug() << "[OSC] [OSCREVEIVER] -> Msg Snd: Group, Key: Value:" << oscIn.oscGroup
                     << "," << oscIn.oscKey << ":" << proxy->getParameter();
        }
    }
}

// OSC wants info from Mixxx -> Value
void OscReceiver::doGetV(OscResult& oscIn) {
    if (sDebug) {
        qDebug() << "[OSC] [OSCREVEIVER] -> doGetV triggered oscIn.oscGroup" << oscIn.oscGroup
                 << " oscIn.oscKey " << oscIn.oscKey;
    }
    if (ControlObject::exists(ConfigKey(oscIn.oscGroup, oscIn.oscKey))) {
        auto proxy = std::make_unique<PollingControlProxy>(oscIn.oscGroup, oscIn.oscKey);
        // for future use when prefix /cop is introduced in osc-messages
        // oscIn.oscGroup = QString("%1%2").arg("/cov", oscIn.oscGroup);
        oscFunctionsSendPtrType(m_pConfig,
                oscIn.oscGroup,
                oscIn.oscKey,
                DefOscBodyType::FLOATBODY,
                "",
                0,
                0,
                static_cast<float>(proxy->get()));
        // static_cast<float>(ControlObject::getControl(
        //         oscIn.oscGroup, oscIn.oscKey)
        //                 ->get()));
        if (sDebug) {
            qDebug() << "[OSC] [OSCREVEIVER] -> Msg Rcvd: Get Group, Key: Value:" << oscIn.oscGroup
                     << "," << oscIn.oscKey << ":" << oscIn.oscValue;
        }
    }
}

void OscReceiver::doGetT(OscResult& oscIn) {
    if (sDebug) {
        qDebug() << "[OSC] [OSCREVEIVER] -> doGetT triggered oscIn.oscGroup" << oscIn.oscGroup
                 << " oscIn.oscKey " << oscIn.oscKey;
    }

    QString searchOscKey = QString(oscIn.oscGroup + oscIn.oscKey);
    if (sDebug) {
        qDebug() << "[OSC] [OSCREVEIVER] -> Msg Rcvd: Get Group, TrackInfo: " << oscIn.oscGroup
                 << "," << oscIn.oscKey;
    }

    // Lock the mutex to protect access to m_pConfig
    QMutexLocker<QMutex> locker(&m_mutex);

    // Read the value from m_pConfig
    const QString sendOscValue = m_pConfig->getValue(ConfigKey("[OSC]", searchOscKey));

    // Send the OSC message using the stored value
    oscFunctionsSendPtrType(m_pConfig,
            oscIn.oscGroup,
            oscIn.oscKey,
            DefOscBodyType::STRINGBODY,
            escapeStringToJsonUnicode(sendOscValue),
            0,
            0,
            0);

    if (sDebug) {
        qDebug() << "[OSC] [OSCREVEIVER] -> Msg Rcvd: Get TrackInfo, Key: Value:" << oscIn.oscGroup
                 << "," << oscIn.oscKey << ":" << sendOscValue;
    }
}

// test readlocjer
// void OscReceiver::doGetT(OscResult& oscIn) {
//    if (sDebug) {
//        qDebug() << "[OSC] [OSCREVEIVER] -> doGetT triggered oscIn.oscGroup"
//        << oscIn.oscGroup
//                 << " oscIn.oscKey " << oscIn.oscKey;
//    }
//    QString searchOscKey = QString(oscIn.oscGroup + oscIn.oscKey);
//    if (sDebug) {
//        qDebug() << "[OSC] [OSCREVEIVER] -> Msg Rcvd: Get Group, TrackInfo: "
//        << oscIn.oscGroup
//                 << "," << oscIn.oscKey;
//    }
//    // Lock the read-write lock for reading
//    QReadLocker locker(&m_configLock);
//
//    // Read the value from m_pConfig
//    const QString sendOscValue = m_pConfig->getValue(ConfigKey("[OSC]",
//    searchOscKey));
//
//    // Unlock the read-write lock immediately after reading
//    locker.unlock();
//
//    // Send the OSC message using the stored value
//    oscFunctionsSendPtrType(m_pConfig,
//            oscIn.oscGroup,
//            oscIn.oscKey,
//            DefOscBodyType::STRINGBODY,
//            escapeStringToJsonUnicode(sendOscValue),
//            0,
//            0,
//            0);
//
//    if (sDebug) {
//        qDebug() << "[OSC] [OSCREVEIVER] -> Msg Rcvd: Get TrackInfo, Key:
//        Value:" << oscIn.oscGroup
//                 << "," << oscIn.oscKey << ":" << sendOscValue;
//    }
//}

// void OscReceiver::doGetT(OscResult& oscIn) {
//     if (sDebug) {
//         qDebug() << "[OSC] [OSCREVEIVER] -> doGetT triggered oscIn.oscGroup"
//         << oscIn.oscGroup
//                  << " oscIn.oscKey " << oscIn.oscKey;
//     }
//     QString searchOscKey = QString(oscIn.oscGroup + oscIn.oscKey);
//     if (sDebug) {
//         qDebug() << "[OSC] [OSCREVEIVER] -> Msg Rcvd: Get Group, TrackInfo: "
//         << oscIn.oscGroup
//                  << "," << oscIn.oscKey;
//     }
//     // if (!m_pConfig->getValue(ConfigKey("[OSC]", searchOscKey)) {
//     const QString& sendOscValue = m_pConfig->getValue(ConfigKey("[OSC]",
//     searchOscKey));
//     // for future use when prefix /cop is introduced in osc-messages
//     // oscIn.oscGroup = QString("%1%2").arg("/cot", oscIn.oscGroup);
//      oscFunctionsSendPtrType(m_pConfig,
//            oscIn.oscGroup,
//            oscIn.oscKey,
//            DefOscBodyType::STRINGBODY,
//            escapeStringToJsonUnicode(sendOscValue),
//            0,
//            0,
//            0);
//     //}
//     if (sDebug) {
//         qDebug() << "[OSC] [OSCREVEIVER] -> Msg Rcvd: Get TrackInfo, Key:
//         Value:" << oscIn.oscGroup
//                  << "," << oscIn.oscKey << ":" << sendOscValue;
//     }
// }

// Input from OSC -> Changes in Mixxx
void OscReceiver::doSet(OscResult& oscIn, float value) {
    if (sDebug) {
        qDebug() << "[OSC] [OSCREVEIVER] -> doSet triggered oscIn.oscGroup" << oscIn.oscGroup
                 << " oscIn.oscKey " << oscIn.oscKey;
    }
    if (ControlObject::exists(ConfigKey(oscIn.oscGroup, oscIn.oscKey))) {
        auto proxy = std::make_unique<PollingControlProxy>(oscIn.oscGroup, oscIn.oscKey);
        proxy->set(value);
        oscFunctionsSendPtrType(m_pConfig,
                oscIn.oscGroup,
                oscIn.oscKey,
                DefOscBodyType::FLOATBODY,
                "",
                0,
                0,
                value);
        if (sDebug) {
            qDebug() << "[OSC] [OSCREVEIVER] -> Msg Rcvd: Group, Key: Value:" << oscIn.oscGroup
                     << "," << oscIn.oscKey << ":" << value;
        }
    } else {
        if (sDebug) {
            qDebug() << "[OSC] [OSCREVEIVER] -> Msg Rcvd for non-existing Control Object: Group, "
                        "Key: Value:"
                     << oscIn.oscGroup << "," << oscIn.oscKey << ":" << value;
        }
    }
}

// trigger OSC to sync
void OscReceiver::sendOscSyncTriggers() {
    if (sDebug) {
        qDebug() << "[OSC] [OSCREVEIVER] -> Mixxx OSC SendSyncTrigger";
    }
    int interval = m_pConfig
                           ->getValue(ConfigKey(
                                   "[OSC]", "OscSendSyncTriggersInterval"))
                           .toInt() /
            1000;
    int checkStamp = QDateTime::currentDateTime().toString("ss").toInt();
    if (sDebug) {
        qDebug() << "[OSC] [OSCREVEIVER] -> Mixxx OSC SENT SendSyncTrigger: "
                    "checkStamp:"
                 << checkStamp;
    }
    if (checkStamp % interval == 0) {
        oscFunctionsSendPtrType(m_pConfig,
                "[Osc]",
                "OscSync",
                DefOscBodyType::FLOATBODY,
                "",
                0,
                0,
                1);
        if (sDebug) {
            qDebug() << "[OSC] [OSCREVEIVER] -> Mixxx OSC SENT SendSyncTrigger";
        }
    }
}

// with QT Event Loop
// void OscReceiver::startOscReceiver(int oscPortin) {
//    std::string portStr = std::to_string(oscPortin);
//
//    // Create a new OSC server (not a server thread)
//    lo_server server = lo_server_new_with_proto(portStr.c_str(), LO_UDP,
//    errorCallback); if (!server) {
//        qWarning() << "[OSC] [OSCREVEIVER] -> Failed to create OSC server.";
//        return;
//    }
//
//    // Add methods to the server
//    lo_server_add_method(server, "/quit", "", quit_handler, nullptr);
//    lo_server_add_method(server, nullptr, nullptr, messageCallback, this); //
//    Pass `this` as user_data
//
//    qDebug() << "[OSC] Receiver started and awaiting messages...";
//
//    // Use a QTimer to periodically process OSC messages
//    QTimer* timer = new QTimer(this);
//    connect(timer, &QTimer::timeout, this, [server]() {
//        // Process OSC messages (non-blocking)
//        lo_server_recv_noblock(server, 0);
//    });
//    timer->start(100); // Process messages every 100ms
//}

// liblo without own thread
// int OscReceiver::startOscReceiver(int oscPortin, UserSettingsPointer m_pConfig) {
// int OscReceiver::startOscReceiver(int oscPortin) {
//    std::string portStr = std::to_string(oscPortin);
//
//    // Create a new OSC server (not a server thread)
//    lo_server server = lo_server_new_with_proto(portStr.c_str(), LO_UDP, errorCallback);
//    if (!server) {
//        qWarning() << "[OSC] [OSCREVEIVER] -> Failed to create OSC server.";
//        return -1;
//    }
//    // Add methods to the server
//    lo_server_add_method(server, "/quit", "", quit_handler, nullptr);
//    lo_server_add_method(server,
//            nullptr,
//            nullptr,
//            messageCallback,
//            this); // Pass `this` as user_data
//    qDebug() << "[OSC] Receiver started and awaiting messages...";
//    // Main loop to process OSC messages
//    while (!m_stopFlag) {
//        // Process OSC messages (non-blocking)
//        int timeout_ms = 100; // Wait for 100ms for incoming messages
//        lo_server_recv_noblock(server, timeout_ms);
//    }
//    // Stop the server when done
//    lo_server_free(server);
//
//    qDebug() << "[OSC] [OSCREVEIVER] -> Receiver stopped";
//}

// liblo in own thread
// int OscReceiver::startOscReceiver(int oscPortin, UserSettingsPointer m_pConfig) {
int OscReceiver::startOscReceiver(int oscPortin) {
    std::string portStr = std::to_string(oscPortin);
    lo_server_thread st = lo_server_thread_new_with_proto(portStr.c_str(), LO_UDP, errorCallback);
    lo_server s = lo_server_thread_get_server(st);
    lo_server_thread_add_method(st, "/quit", "", quit_handler, NULL);
    lo_server_thread_add_method(st, NULL, NULL, messageCallback, s);
    lo_server_thread_start(st);
    // lo_address a = 0;
    // a = lo_address_new_with_proto(LO_UDP, "192.168.0.125", "9000");
    // if (!a) {
    //     qDebug() << "EVE -> LIBLO -> Error creating destination address.\n";
    //     exit(1);
    // }
    // qDebug() << "EVE -> LIBLO -> Sending message to " << a;
    // int r = lo_send_from(a, s, LO_TT_IMMEDIATE, "/test", "ifs", 1, 2.0f, "3");
    // if (r < 0)
    //     qDebug() << "EVE -> LIBLO -> Error sending initial message.\n";
    qDebug() << "[OSC] Receiver started and awaiting messages...";

    while (!m_stopFlag) {
        QThread::msleep(100); // Sleep for a short period
    }

    // while (true) {
    //     // mutexlocker added for safe adding flag
    //     QMutexLocker locker(&m_mutex);
    //     if (m_stopFlag) {
    //         break; // If the stop flag is set, exit the loop
    //     }

    //    // Sleep for a short period before checking the flag again
    //    QThread::msleep(100);
    //}

    // Stop the server when done
    lo_server_thread_stop(st);
    lo_server_thread_free(st);

    qDebug() << "[OSC] [OSCREVEIVER] -> Receiver stopped";

    return 0;
}

void OscReceiver::oscReceiverMain(UserSettingsPointer pConfig) {
    if (pConfig->getValue<bool>(ConfigKey("[OSC]", "OscEnabled"))) {
        if (sDebug) {
            qDebug() << "[OSC] [OSCREVEIVER] -> Enabled -> Started";
        }

        // Check which receivers are activated in prefs
        for (int i = 1; i <= 5; ++i) {
            QString receiverActive = QString("OscReceiver%1Active").arg(i);
            QString receiverIp = QString("OscReceiver%1Ip").arg(i);
            if (pConfig->getValue<bool>(ConfigKey("[OSC]", receiverActive))) {
                const QString& ckOscRecIp = pConfig->getValue(ConfigKey("[OSC]", receiverIp));
                if (sDebug) {
                    qDebug() << QString(
                            "[OSC] [OSCREVEIVER] -> Mixxx OSC Receiver %1 with "
                            "ip-address: %2 Activated")
                                        .arg(i)
                                        .arg(ckOscRecIp);
                }
            } else {
                if (sDebug) {
                    qDebug()
                            << QString("[OSC] [OSCREVEIVER] -> Mixxx OSC "
                                       "Receiver %1 Not Activated")
                                       .arg(i);
                }
            }
        }
    } else {
        if (sDebug) {
            qDebug() << "[OSC] [OSCREVEIVER] -> Mixxx OSC Service NOT Enabled";
        }
    }
}

//
// void OscReceiver::oscReceiverMain(UserSettingsPointer pConfig) {
//    //    //if (!pConfig) {
//    //    //    qWarning() << "[OSC] [OSCRECEIVER] -> Config not loaded yet.
//    Delaying OSC startup...";
//    //    //    QTimer::singleShot(1000, this, [this, pConfig]() {
//    oscReceiverMain(pConfig); });
//    //    //    return;
//    //    //}
//    if (pConfig->getValue<bool>(ConfigKey("[OSC]", "OscEnabled"))) {
//        int ckOscPortInInt = pConfig->getValue(ConfigKey("[OSC]",
//        "OscPortIn")).toInt(); if (sDebug) {
//            qDebug() << "[OSC] [OSCREVEIVER] -> Enabled -> Started";
//        }
//        // Check which receivers are activated in prefs
//        for (int i = 1; i <= 5; ++i) {
//            QString receiverActive = QString("OscReceiver%1Active").arg(i);
//            QString receiverIp = QString("OscReceiver%1Ip").arg(i);
//            if (pConfig->getValue<bool>(ConfigKey("[OSC]", receiverActive))) {
//                const QString& ckOscRecIp =
//                pConfig->getValue(ConfigKey("[OSC]", receiverIp)); if (sDebug)
//                {
//                    qDebug() << QString("[OSC] [OSCREVEIVER] -> Mixxx OSC
//                    Receiver %1 with ip-address: %2
//                    Activated").arg(i).arg(ckOscRecIp);
//                }
//            } else {
//                if (sDebug) {
//                    qDebug() << QString("[OSC] [OSCREVEIVER] -> Mixxx OSC
//                    Receiver %1 Not Activated").arg(i);
//                }
//            }
//        }
//        // Start thread
//        if (sDebug) {
//            qDebug() << "[OSC] [OSCREVEIVER] -> Mixxx OSC Service Thread
//            starting";
//        }
//        // Create the receiver
//        OscReceiver* oscReceiver = new OscReceiver(pConfig);
//        // Create the thread
//        QThread* oscThread = new QThread();
//        oscReceiver->moveToThread(oscThread);
//        // thread -> sifnal to startOscReceiver()
//        QObject::connect(oscThread, &QThread::started, oscReceiver,
//        [oscReceiver, ckOscPortInInt]() {
//        oscReceiver->startOscReceiver(ckOscPortInInt);
//        });
//
//        // Connect the thread finished signal to delete oscReceiver after it's
//        finished QObject::connect(oscThread, &QThread::finished, oscReceiver,
//        &QObject::deleteLater);
//        // Connect the thread finished signal to stop the thread safely
//        QObject::connect(oscThread, &QThread::finished, oscThread,
//        &QThread::deleteLater);
//        // Start the thread
//        oscThread->start();
//
//        if (sDebug) {
//            qDebug() << "[OSC] [OSCREVEIVER] -> Mixxx OSC Service Thread
//            quit";
//        }
//    } else {
//        if (sDebug) {
//            qDebug() << "[OSC] [OSCREVEIVER] -> Mixxx OSC Service NOT
//            Enabled";
//        }
//    }
//}
