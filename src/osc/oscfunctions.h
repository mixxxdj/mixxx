#ifndef OSCFUNCTIONS_H
#define OSCFUNCTIONS_H

constexpr int OUTPUT_BUFFER_SIZE = 1024;
constexpr int IP_MTU_SIZE = 1536;

#include <QDataStream>
#include <QFile>
#include <QSharedPointer>
#include <iostream>
#include <memory>

#include "osc/ip/UdpSocket.h"
#include "osc/osc/OscOutboundPacketStream.h"
#include "osc/osc/OscPacketListener.h"
#include "osc/osc/OscReceivedElements.h"

enum DefOscBodyType {
    STRINGBODY = 1,
    INTBODY = 2,
    DOUBLEBODY = 3,
    FLOATBODY = 4
};

void OscFunctionsSendPtrType(UserSettingsPointer m_pConfig,
        QString OscGroup,
        QString OscKey,
        enum DefOscBodyType OscBodyType,
        QString OscMessageBodyQString,
        int OscMessageBodyInt,
        double OscMessageBodyDouble,
        float OscMessageBodyFloat) {
    QString OscMessageHeader = "/" + OscGroup + "@" + OscKey;
    OscMessageHeader.replace("[", "(");
    OscMessageHeader.replace("]", ")");

    QByteArray OscMessageHeaderBa = OscMessageHeader.toLocal8Bit();
    const char* OscMessageHeaderChar = OscMessageHeaderBa.data();
    QByteArray OscMessageBodyBa;
    const char* OscMessageBodyChar;

    if (m_pConfig->getValue<bool>(ConfigKey("[OSC]", "OscEnabled"))) {
        char buffer[IP_MTU_SIZE];
        osc::OutboundPacketStream p(buffer, IP_MTU_SIZE);
        QString MixxxOSCStatusTxtBody;
        switch (OscBodyType) {
        case 1: // QSTRINGBODY = 1
            OscMessageBodyBa = OscMessageBodyQString.toLocal8Bit();
            OscMessageBodyChar = OscMessageBodyBa.data();
            p.Clear();
            p << osc::BeginBundle();
            p << osc::BeginMessage(OscMessageHeaderChar) << OscMessageBodyChar << osc::EndMessage;
            p << osc::EndBundle;
            MixxxOSCStatusTxtBody = OscMessageBodyChar;
            break;
        case 2: // INTBODY = 2
            p.Clear();
            p << osc::BeginBundle();
            p << osc::BeginMessage(OscMessageHeaderChar) << OscMessageBodyInt << osc::EndMessage;
            p << osc::EndBundle;
            MixxxOSCStatusTxtBody = QString::number(OscMessageBodyInt);
            break;
        case 3: // DOUBLEBODY = 3
            p.Clear();
            p << osc::BeginBundle();
            p << osc::BeginMessage(OscMessageHeaderChar) << OscMessageBodyDouble << osc::EndMessage;
            p << osc::EndBundle;
            MixxxOSCStatusTxtBody = QString::number(OscMessageBodyDouble);
            break;
        case 4: // FLOATBODY = 4
            p.Clear();
            p << osc::BeginBundle();
            p << osc::BeginMessage(OscMessageHeaderChar) << OscMessageBodyFloat << osc::EndMessage;
            p << osc::EndBundle;
            MixxxOSCStatusTxtBody = QString::number(OscMessageBodyFloat);
            break;
        }

        QString CKOscPortOut = m_pConfig->getValue(ConfigKey("[OSC]", "OscPortOut"));
        int CKOscPortOutInt = CKOscPortOut.toInt();
        if (m_pConfig->getValue<bool>(ConfigKey("[OSC]", "OscReceiver1Active"))) {
            //            QString CKOscRec1Active =
            //            m_pConfig->getValue(ConfigKey("[OSC]",
            //            "OscReceiver1Active"));
            QString CKOscRec1Ip = m_pConfig->getValue(ConfigKey("[OSC]", "OscReceiver1Ip"));
            QByteArray CKOscRec1Ipba = CKOscRec1Ip.toLocal8Bit();
            const char* CKOscRec1IpChar = CKOscRec1Ipba.data();

            UdpTransmitSocket transmitSocket(IpEndpointName(CKOscRec1IpChar, CKOscPortOutInt));
            transmitSocket.Send(p.Data(), p.Size());
            qDebug() << "OSC Msg Send to Receiver 1 ("
                     << CKOscRec1IpChar << ":" << CKOscPortOutInt
                     << QString(") : <%1 : %2")
                                .arg(OscMessageHeader)
                                .arg(MixxxOSCStatusTxtBody);
        }
        if (m_pConfig->getValue<bool>(ConfigKey("[OSC]", "OscReceiver2Active"))) {
            //          QString CKOscRec2Active =
            //          m_pConfig->getValue(ConfigKey("[OSC]",
            //          "OscReceiver2Active"));
            QString CKOscRec2Ip = m_pConfig->getValue(ConfigKey("[OSC]", "OscReceiver2Ip"));
            QByteArray CKOscRec2Ipba = CKOscRec2Ip.toLocal8Bit();
            const char* CKOscRec2IpChar = CKOscRec2Ipba.data();

            UdpTransmitSocket transmitSocket(IpEndpointName(CKOscRec2IpChar, CKOscPortOutInt));
            transmitSocket.Send(p.Data(), p.Size());
            qDebug() << "OSC Msg Send to Receiver 2 ("
                     << CKOscRec2IpChar << ":" << CKOscPortOutInt
                     << QString(") : <%1 : %2")
                                .arg(OscMessageHeader)
                                .arg(MixxxOSCStatusTxtBody);
        }
        if (m_pConfig->getValue<bool>(ConfigKey("[OSC]", "OscReceiver3Active"))) {
            //            QString CKOscRec3Active =
            //            m_pConfig->getValue(ConfigKey("[OSC]",
            //            "OscReceiver3Active"));
            QString CKOscRec3Ip = m_pConfig->getValue(ConfigKey("[OSC]", "OscReceiver3Ip"));
            QByteArray CKOscRec3Ipba = CKOscRec3Ip.toLocal8Bit();
            const char* CKOscRec3IpChar = CKOscRec3Ipba.data();

            UdpTransmitSocket transmitSocket(IpEndpointName(CKOscRec3IpChar, CKOscPortOutInt));
            transmitSocket.Send(p.Data(), p.Size());
            qDebug() << "OSC Msg Send to Receiver 3 ("
                     << CKOscRec3IpChar << ":" << CKOscPortOutInt
                     << QString(") : <%1 : %2")
                                .arg(OscMessageHeader)
                                .arg(MixxxOSCStatusTxtBody);
        }
        if (m_pConfig->getValue<bool>(ConfigKey("[OSC]", "OscReceiver4Active"))) {
            //            QString CKOscRec4Active =
            //            m_pConfig->getValue(ConfigKey("[OSC]",
            //            "OscReceiver4Active"));
            QString CKOscRec4Ip = m_pConfig->getValue(ConfigKey("[OSC]", "OscReceiver4Ip"));
            QByteArray CKOscRec4Ipba = CKOscRec4Ip.toLocal8Bit();
            const char* CKOscRec4IpChar = CKOscRec4Ipba.data();

            UdpTransmitSocket transmitSocket(IpEndpointName(CKOscRec4IpChar, CKOscPortOutInt));
            qDebug() << "OSC Msg Send to Receiver 4 ("
                     << CKOscRec4IpChar << ":" << CKOscPortOutInt
                     << QString(") : <%1 : %2")
                                .arg(OscMessageHeader)
                                .arg(MixxxOSCStatusTxtBody);
        }
        if (m_pConfig->getValue<bool>(ConfigKey("[OSC]", "OscReceiver5Active"))) {
            //            QString CKOscRec5Active =
            //            m_pConfig->getValue(ConfigKey("[OSC]",
            //            "OscReceiver5Active"));
            QString CKOscRec5Ip = m_pConfig->getValue(ConfigKey("[OSC]", "OscReceiver5Ip"));
            QByteArray CKOscRec5Ipba = CKOscRec5Ip.toLocal8Bit();
            const char* CKOscRec5IpChar = CKOscRec5Ipba.data();

            UdpTransmitSocket transmitSocket(IpEndpointName(CKOscRec5IpChar, CKOscPortOutInt));
            transmitSocket.Send(p.Data(), p.Size());
            qDebug() << "OSC Msg Send to Receiver 5 ("
                     << CKOscRec5IpChar << ":" << CKOscPortOutInt
                     << QString(") : <%1 : %2")
                                .arg(OscMessageHeader)
                                .arg(MixxxOSCStatusTxtBody);
        }
    } else {
        qDebug() << "OSC NOT Enabled";
    }
}

#endif /* OSCFUNCTIONS_H */
