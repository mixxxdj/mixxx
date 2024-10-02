#include <mmsystem.h>

#include <QThread>
#include <bitset>
#include <cstdlib>
#include <cstring>
#include <iostream>
// #include <winmm>

// #pragma comment(lib, "winmm.lib")
// #include (lib, "winmm.dll")
#if defined(__BORLANDC__) // workaround for BCB4 release build intrinsics bug
namespace std {
using ::__strcmp__; // avoid error: E2316 '__strcmp__' is not a member of 'std'.
}
#endif

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "control/pollingcontrolproxy.h"
#include "osc/ip/UdpSocket.h"
#include "osc/osc/OscPacketListener.h"
#include "osc/osc/OscReceivedElements.h"
#include "oscfunctions.h"
#include "oscreceiver.h"

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
    void ProcessMessage(const osc::ReceivedMessage& m,
            const IpEndpointName& remoteEndpoint) {
        (void)remoteEndpoint;

        try {
            if (m_pConfig->getValue<bool>(ConfigKey("[OSC]", "OscSendSyncTriggers"))) {
                qDebug() << "Mixxx OSC SendSyncTrigger";
                int interval = (m_pConfig->getValue(ConfigKey("[OSC]",
                                        "OscSendSyncTriggersInterval")))
                                       .toInt() /
                        1000;
                int checkStamp = QDateTime::currentDateTime().toString("ss").toInt();
                qDebug() << "Mixxx OSC SENT SendSyncTrigger: checkStamp: " << checkStamp;
                if (checkStamp % interval == 0) {
                    OscFunctionsSendPtrType(m_pConfig,
                            "[Osc]",
                            "OscSync",
                            FLOATBODY,
                            "",
                            0,
                            0,
                            1);
                    // qDebug() << "Mixxx OSC SENT SendSyncTrigger";
                }
            }

            osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
            //            osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();

            float oscInVal;
            args >> oscInVal >> osc::EndMessage;

            oscResult oscIn;
            oscIn.oscAddress = m.AddressPattern();
            oscIn.oscAddress.replace("/", "");
            oscIn.oscAddress.replace("(", "[");
            oscIn.oscAddress.replace(")", "]");
            oscIn.oscValue = oscInVal;
            bool oscGetP = false;
            bool oscGetV = false;
            bool oscSet = false;

            if (oscIn.oscAddress.contains("GetP#", Qt::CaseSensitive)) {
                int posDel1 = oscIn.oscAddress.indexOf("GetP#", 0, Qt::CaseInsensitive);
                if (posDel1 == 0) {
                    oscGetP = true;
                }
            } else {
                if (oscIn.oscAddress.contains("GetV#", Qt::CaseSensitive)) {
                    int posDel1 = oscIn.oscAddress.indexOf("GetV#", 0, Qt::CaseInsensitive);
                    if (posDel1 == 0) {
                        oscGetV = true;
                    }
                } else {
                    oscSet = true;
                }
            }

            // OSC wants info from Mixxx -> Parameter
            if (oscGetP) {
                int posDel2 = oscIn.oscAddress.indexOf("@", 0, Qt::CaseInsensitive);
                if (posDel2 > 0) {
                    oscIn.oscGroup = oscIn.oscAddress.mid(5, posDel2 - 5);
                    oscIn.oscKey = oscIn.oscAddress.mid(posDel2 + 1, oscIn.oscAddress.length() - 5);
                    qDebug() << "OSC Msg Rcvd: Get Group, Key: Value: "
                             << oscIn.oscGroup << "," << oscIn.oscKey << ":"
                             << oscIn.oscValue;
                    if (ControlObject::exists(ConfigKey(oscIn.oscGroup, oscIn.oscKey))) {
                        std::unique_ptr<PollingControlProxy> m_poscPCP;
                        m_poscPCP = std::make_unique<PollingControlProxy>(
                                oscIn.oscGroup, oscIn.oscKey);
                        // float m_posPCPValue = m_poscPCP->get();
                        // double m_posPCPValue = m_poscPCP->get();
                        OscFunctionsSendPtrType(m_pConfig,
                                oscIn.oscGroup,
                                oscIn.oscKey,
                                FLOATBODY,
                                "",
                                0,
                                0,
                                0 + m_poscPCP->get());
                        qDebug() << "OSC Msg Snd: Group, Key: Value: "
                                 << oscIn.oscGroup << "," << oscIn.oscKey << ":"
                                 // << m_posPCPValue;
                                 << m_poscPCP->get();
                    }
                }
            }

            // OSC wants info from Mixxx -> Value
            if (oscGetV) {
                int posDel2 = oscIn.oscAddress.indexOf("@", 0, Qt::CaseInsensitive);
                if (posDel2 > 0) {
                    oscIn.oscGroup = oscIn.oscAddress.mid(5, posDel2 - 5);
                    oscIn.oscKey = oscIn.oscAddress.mid(posDel2 + 1, oscIn.oscAddress.length() - 5);
                    if (ControlObject::exists(ConfigKey(oscIn.oscGroup, oscIn.oscKey))) {
                        OscFunctionsSendPtrType(m_pConfig,
                                oscIn.oscGroup,
                                oscIn.oscKey,
                                FLOATBODY,
                                "",
                                0,
                                0,
                                ControlObject::getControl(
                                        oscIn.oscGroup, oscIn.oscKey)
                                        ->get());
                    }
                    qDebug() << "OSC Msg Rcvd: Get Group, Key: Value: "
                             << oscIn.oscGroup << "," << oscIn.oscKey << ":"
                             << oscIn.oscValue;
                }
            }

            // Input from OSC -> Changes in Mixxx
            if (!oscGetP && !oscGetV && oscSet) {
                int posDel2 = oscIn.oscAddress.indexOf("@", 0, Qt::CaseInsensitive);
                if (posDel2 > 0) {
                    oscIn.oscGroup = oscIn.oscAddress.mid(0, posDel2);
                    oscIn.oscKey = oscIn.oscAddress.mid(posDel2 + 1, oscIn.oscAddress.length());
                    if (ControlObject::exists(ConfigKey(oscIn.oscGroup, oscIn.oscKey))) {
                        std::unique_ptr<PollingControlProxy> m_poscPCP;
                        m_poscPCP = std::make_unique<PollingControlProxy>(
                                oscIn.oscGroup, oscIn.oscKey);
                        m_poscPCP->set(oscIn.oscValue);
                        // float m_posPCPValue = m_poscPCP->get();
                        OscFunctionsSendPtrType(m_pConfig,
                                oscIn.oscGroup,
                                oscIn.oscKey,
                                FLOATBODY,
                                "",
                                0,
                                0,
                                oscIn.oscValue);
                        qDebug() << "OSC Msg Rcvd: Group, Key: Value: "
                                 << oscIn.oscGroup << "," << oscIn.oscKey << ":"
                                 << oscIn.oscValue;
                    } else {
                        qDebug() << "OSC Msg Rcvd for not existing CO : Group, Key: Value: "
                                 << oscIn.oscGroup << "," << oscIn.oscKey << ":"
                                 << oscIn.oscValue;
                    }
                }
            }
        } catch (osc::Exception& e) {
            qDebug() << "OSC Error parsing Msg from " << m.AddressPattern()
                     << "error: " << e.what();
        }
    }
};

void RunOscReceiver(int OscPortIn, UserSettingsPointer m_pConfig) {
    qDebug() << "Mixxx OSC Service Thread started (RunOscReceiver -> OscReceivePacketListener)";
    OscReceivePacketListener listener(m_pConfig);
    UdpListeningReceiveSocket s(IpEndpointName(IpEndpointName::ANY_ADDRESS, OscPortIn),
            &listener);
    s.Run();
}

// #ifndef NO_OSC_TEST_MAIN

void OscReceiverMain(UserSettingsPointer m_pConfig) {
    if (m_pConfig->getValue<bool>(ConfigKey("[OSC]", "OscEnabled"))) {
        //        QString CKOscPortOut = m_pConfig->getValue(ConfigKey("[OSC]", "OscPortOut"));
        QString CKOscPortIn = m_pConfig->getValue(ConfigKey("[OSC]", "OscPortIn"));
        //        int CKOscPortOutInt = CKOscPortOut.toInt();
        int CKOscPortInInt = CKOscPortIn.toInt();
        qDebug() << "OSC Enabled -> Started";

        if (m_pConfig->getValue<bool>(ConfigKey("[OSC]", "OscReceiver1Active"))) {
            //            QString CKOscRec1Active =
            //            m_pConfig->getValue(ConfigKey("[OSC]",
            //            "OscReceiver1Active"));
            const QString& CKOscRec1Ip = m_pConfig->getValue(ConfigKey("[OSC]", "OscReceiver1Ip"));
            QByteArray CKOscRec1Ipba = CKOscRec1Ip.toLocal8Bit();
            const char* CKOscRec1IpChar = CKOscRec1Ipba.data();
            qDebug() << "Mixxx OSC Receiver 1 Activated, ip-Address : "
                     << CKOscRec1IpChar;
        } else {
            qDebug() << "Mixxx OSC Receiver 1 Not Activated";
        }
        if (m_pConfig->getValue<bool>(ConfigKey("[OSC]", "OscReceiver2Active"))) {
            //            QString CKOscRec2Active =
            //            m_pConfig->getValue(ConfigKey("[OSC]",
            //            "OscReceiver2Active"));
            const QString& CKOscRec2Ip = m_pConfig->getValue(ConfigKey("[OSC]", "OscReceiver2Ip"));
            QByteArray CKOscRec2Ipba = CKOscRec2Ip.toLocal8Bit();
            const char* CKOscRec2IpChar = CKOscRec2Ipba.data();
            qDebug() << "Mixxx OSC Receiver 2 Activated, ip-Address : "
                     << CKOscRec2IpChar;
        } else {
            qDebug() << "Mixxx OSC Receiver 2 Not Activated";
        }
        if (m_pConfig->getValue<bool>(ConfigKey("[OSC]", "OscReceiver3Active"))) {
            //            QString CKOscRec3Active =
            //            m_pConfig->getValue(ConfigKey("[OSC]",
            //            "OscReceiver3Active"));
            const QString& CKOscRec3Ip = m_pConfig->getValue(ConfigKey("[OSC]", "OscReceiver3Ip"));
            QByteArray CKOscRec3Ipba = CKOscRec3Ip.toLocal8Bit();
            const char* CKOscRec3IpChar = CKOscRec3Ipba.data();
            qDebug() << "Mixxx OSC Receiver 3 Activated, ip-Address : "
                     << CKOscRec3IpChar;
        } else {
            qDebug() << "Mixxx OSC Receiver 3 Not Activated";
        }
        if (m_pConfig->getValue<bool>(ConfigKey("[OSC]", "OscReceiver4Active"))) {
            //            QString CKOscRec4Active =
            //            m_pConfig->getValue(ConfigKey("[OSC]",
            //            "OscReceiver4Active"));
            const QString& CKOscRec4Ip = m_pConfig->getValue(ConfigKey("[OSC]", "OscReceiver4Ip"));
            QByteArray CKOscRec4Ipba = CKOscRec4Ip.toLocal8Bit();
            const char* CKOscRec4IpChar = CKOscRec4Ipba.data();
            qDebug() << "Mixxx OSC Receiver 4 Activated, ip-Address : "
                     << CKOscRec4IpChar;
        } else {
            qDebug() << "Mixxx OSC Receiver 4 Not Activated";
        }
        if (m_pConfig->getValue<bool>(ConfigKey("[OSC]", "OscReceiver5Active"))) {
            //            QString CKOscRec5Active =
            //            m_pConfig->getValue(ConfigKey("[OSC]",
            //            "OscReceiver5Active"));
            const QString& CKOscRec5Ip = m_pConfig->getValue(ConfigKey("[OSC]", "OscReceiver5Ip"));
            QByteArray CKOscRec5Ipba = CKOscRec5Ip.toLocal8Bit();
            const char* CKOscRec5IpChar = CKOscRec5Ipba.data();
            qDebug() << "Mixxx OSC Receiver 5 Activated, ip-Address : "
                     << CKOscRec5IpChar;
        } else {
            qDebug() << "Mixxx OSC Receiver 5 Not Activated";
        }

        //        QString OscTrackGroup;
        for (int i = 1; i < 5; i++) {
            const QString& OscTrackGroup = QString("[Channel%1]").arg(i);
            OscFunctionsSendPtrType(m_pConfig,
                    OscTrackGroup,
                    "TrackArtist",
                    STRINGBODY,
                    "no track loaded",
                    0,
                    0,
                    0);
            OscFunctionsSendPtrType(m_pConfig,
                    OscTrackGroup,
                    "TrackTitle",
                    STRINGBODY,
                    "no track loaded",
                    0,
                    0,
                    0);
            OscFunctionsSendPtrType(m_pConfig,
                    OscTrackGroup,
                    "duration",
                    FLOATBODY,
                    "",
                    0,
                    0,
                    0);
            OscFunctionsSendPtrType(m_pConfig,
                    OscTrackGroup,
                    "track_loaded",
                    FLOATBODY,
                    "",
                    0,
                    0,
                    0);
        }

        qDebug() << "Mixxx OSC Service Thread starting";
        std::thread tosc(RunOscReceiver, CKOscPortInInt, m_pConfig);
        tosc.detach();
        qDebug() << "Mixxx OSC Service Thread quit";
    } else {
        qDebug() << "Mixxx OSC Service NOT Enabled";
    }
}

// #endif /* NO_OSC_TEST_MAIN */
