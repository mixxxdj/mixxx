#include <QThread>
#include <cstdlib>
#include <cstring>
#include <iostream>

#pragma comment(lib, "winmm.lib")

#include "control/controlobject.h"
#include "osc/ip/UdpSocket.h"
#include "osc/osc/OscPacketListener.h"
#include "osc/osc/OscReceivedElements.h"
#include "oscfunctions.h"
#include "oscreceiver.h"

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
            osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
            osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();

            float oscInVal;
            args >> oscInVal >> osc::EndMessage;

            oscResult oscIn;
            oscIn.oscAddress = m.AddressPattern();
            oscIn.oscGroup, oscIn.oscKey;
            oscIn.oscAddress.replace("/", "");
            oscIn.oscValue = oscInVal;
            int posDel = oscIn.oscAddress.indexOf("@", 0, Qt::CaseInsensitive);
            if (posDel > 0) {
                oscIn.oscGroup = oscIn.oscAddress.mid(0, posDel);
                oscIn.oscGroup = "[" + oscIn.oscGroup + "]";
                oscIn.oscKey = oscIn.oscAddress.mid(posDel + 1, oscIn.oscAddress.length());

                QString MixxxOSCStatusFileLocation =
                        m_pConfig->getSettingsPath() + "/MixxxOSCStatus.txt";
                QFile MixxxOSCStatusFile(MixxxOSCStatusFileLocation);
                MixxxOSCStatusFile.open(QIODevice::ReadWrite | QIODevice::Append);
                QTextStream MixxxOSCStatusTxt(&MixxxOSCStatusFile);
                MixxxOSCStatusTxt << QString("OSC Msg Rcvd: Group, Key: Value: "
                                             "<%1,%2 : %3>")
                                             .arg(oscIn.oscGroup)
                                             .arg(oscIn.oscKey)
                                             .arg(oscIn.oscValue)
                                  << "\n";
                ControlObject::getControl(oscIn.oscGroup, oscIn.oscKey)->set(oscIn.oscValue);
                MixxxOSCStatusFile.close();
                qDebug() << "OSC Msg Rcvd: Group, Key: Value: "
                         << oscIn.oscGroup << "," << oscIn.oscKey << ":"
                         << oscIn.oscValue;
            };

        } catch (osc::Exception& e) {
            // std::cout << "error while parsing message: " <<
            // m.AddressPattern() << ": " << e.what() << "\n";
            QString MixxxOSCStatusFileLocation =
                    m_pConfig->getSettingsPath() + "/MixxxOSCStatus.txt";
            QFile MixxxOSCStatusFile(MixxxOSCStatusFileLocation);
            MixxxOSCStatusFile.open(QIODevice::ReadWrite | QIODevice::Append);
            QTextStream MixxxOSCStatusTxt(&MixxxOSCStatusFile);
            MixxxOSCStatusTxt << QString("OSC Error parsing Msg from %1 : %2").arg(m.AddressPattern()).arg(e.what()) << "\n";
            MixxxOSCStatusFile.close();
            qDebug() << "OSC Error parsing Msg from " << m.AddressPattern() << "error: " << e.what();
        }
    }
};

void RunOscReceiver(int OscPortIn, UserSettingsPointer m_pConfig) {
    OscReceivePacketListener listener(m_pConfig);
    UdpListeningReceiveSocket s(IpEndpointName(IpEndpointName::ANY_ADDRESS, OscPortIn),
            &listener);
    s.Run();
}

// #ifndef NO_OSC_TEST_MAIN

void OscReceiverMain(UserSettingsPointer m_pConfig) {
    QString MixxxOSCStatusFileLocation = m_pConfig->getSettingsPath() + "/MixxxOSCStatus.txt";
    QFile MixxxOSCStatusFile(MixxxOSCStatusFileLocation);
    MixxxOSCStatusFile.remove();
    MixxxOSCStatusFile.open(QIODevice::ReadWrite | QIODevice::Append);
    QTextStream MixxxOSCStatusTxt(&MixxxOSCStatusFile);
    if (m_pConfig->getValue<bool>(ConfigKey("[OSC]", "OscEnabled"))) {
        QString CKOscPortOut = m_pConfig->getValue(ConfigKey("[OSC]", "OscPortOut"));
        QString CKOscPortIn = m_pConfig->getValue(ConfigKey("[OSC]", "OscPortIn"));
        int CKOscPortOutInt = CKOscPortOut.toInt();
        int CKOscPortInInt = CKOscPortIn.toInt();
        MixxxOSCStatusTxt << QString("OSC Enabled -> Started") << "\n";
        qDebug() << "OSC Enabled -> Started";
        MixxxOSCStatusTxt << QString("OSC Settings: PortIn            : %1")
                                     .arg(CKOscPortInInt)
                          << "\n";
        MixxxOSCStatusTxt << QString("OSC Settings: PortOut           : %1")
                                     .arg(CKOscPortOutInt)
                          << "\n";

        if (m_pConfig->getValue<bool>(ConfigKey("[OSC]", "OscReceiver1Active"))) {
            QString CKOscRec1Active = m_pConfig->getValue(ConfigKey("[OSC]", "OscReceiver1Active"));
            QString CKOscRec1Ip = m_pConfig->getValue(ConfigKey("[OSC]", "OscReceiver1Ip"));
            QByteArray CKOscRec1Ipba = CKOscRec1Ip.toLocal8Bit();
            const char* CKOscRec1IpChar = CKOscRec1Ipba.data();
            MixxxOSCStatusTxt << QString("OSC Settings: Receiver 1 active : %1")
                                         .arg(CKOscRec1Active)
                              << "\n";
            MixxxOSCStatusTxt << QString("OSC Settings: Receiver 1 ip     : %1")
                                         .arg(CKOscRec1IpChar)
                              << "\n";
        } else {
            MixxxOSCStatusTxt << QString("OSC Settings: Receiver 1 NOT active") << "\n";
        }
        if (m_pConfig->getValue<bool>(ConfigKey("[OSC]", "OscReceiver2Active"))) {
            QString CKOscRec2Active = m_pConfig->getValue(ConfigKey("[OSC]", "OscReceiver2Active"));
            QString CKOscRec2Ip = m_pConfig->getValue(ConfigKey("[OSC]", "OscReceiver2Ip"));
            QByteArray CKOscRec2Ipba = CKOscRec2Ip.toLocal8Bit();
            const char* CKOscRec2IpChar = CKOscRec2Ipba.data();
            MixxxOSCStatusTxt << QString("OSC Settings: Receiver 2 active : %1")
                                         .arg(CKOscRec2Active)
                              << "\n";
            MixxxOSCStatusTxt << QString("OSC Settings: Receiver 2 ip     : %1")
                                         .arg(CKOscRec2IpChar)
                              << "\n";
        } else {
            MixxxOSCStatusTxt << QString("OSC Settings: Receiver 2 NOT active") << "\n";
        }
        if (m_pConfig->getValue<bool>(ConfigKey("[OSC]", "OscReceiver3Active"))) {
            QString CKOscRec3Active = m_pConfig->getValue(ConfigKey("[OSC]", "OscReceiver3Active"));
            QString CKOscRec3Ip = m_pConfig->getValue(ConfigKey("[OSC]", "OscReceiver3Ip"));
            QByteArray CKOscRec3Ipba = CKOscRec3Ip.toLocal8Bit();
            const char* CKOscRec3IpChar = CKOscRec3Ipba.data();
            MixxxOSCStatusTxt << QString("OSC Settings: Receiver 3 active : %1")
                                         .arg(CKOscRec3Active)
                              << "\n";
            MixxxOSCStatusTxt << QString("OSC Settings: Receiver 3 ip     : %1")
                                         .arg(CKOscRec3IpChar)
                              << "\n";
        } else {
            MixxxOSCStatusTxt << QString("OSC Settings: Receiver 3 NOT active") << "\n";
        }
        if (m_pConfig->getValue<bool>(ConfigKey("[OSC]", "OscReceiver4Active"))) {
            QString CKOscRec4Active = m_pConfig->getValue(ConfigKey("[OSC]", "OscReceiver4Active"));
            QString CKOscRec4Ip = m_pConfig->getValue(ConfigKey("[OSC]", "OscReceiver4Ip"));
            QByteArray CKOscRec4Ipba = CKOscRec4Ip.toLocal8Bit();
            const char* CKOscRec4IpChar = CKOscRec4Ipba.data();
            MixxxOSCStatusTxt << QString("OSC Settings: Receiver 4 active : %1")
                                         .arg(CKOscRec4Active)
                              << "\n";
            MixxxOSCStatusTxt << QString("OSC Settings: Receiver 4 ip     : %1")
                                         .arg(CKOscRec4IpChar)
                              << "\n";
        } else {
            MixxxOSCStatusTxt << QString("OSC Settings: Receiver 4 NOT active") << "\n";
        }
        if (m_pConfig->getValue<bool>(ConfigKey("[OSC]", "OscReceiver5Active"))) {
            QString CKOscRec5Active = m_pConfig->getValue(ConfigKey("[OSC]", "OscReceiver5Active"));
            QString CKOscRec5Ip = m_pConfig->getValue(ConfigKey("[OSC]", "OscReceiver5Ip"));
            QByteArray CKOscRec5Ipba = CKOscRec5Ip.toLocal8Bit();
            const char* CKOscRec5IpChar = CKOscRec5Ipba.data();
            MixxxOSCStatusTxt << QString("OSC Settings: Receiver 5 active : %1")
                                         .arg(CKOscRec5Active)
                              << "\n";
            MixxxOSCStatusTxt << QString("OSC Settings: Receiver 5 ip     : %1")
                                         .arg(CKOscRec5IpChar)
                              << "\n";
        } else {
            MixxxOSCStatusTxt << QString("OSC Settings: Receiver 5 NOT active") << "\n";
        }
        std::thread tosc(RunOscReceiver, CKOscPortInInt, m_pConfig);
        tosc.detach();
    } else {
        MixxxOSCStatusTxt << QString("OSC NOT Enabled") << "\n";
    }
    MixxxOSCStatusFile.close();
}

// #endif /* NO_OSC_TEST_MAIN */
