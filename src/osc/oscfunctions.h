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

enum class DefOscBodyType {
    STRINGBODY,
    INTBODY,
    DOUBLEBODY,
    FLOATBODY
};

void sendOscMessage(const char* receiverIp,
        int port,
        osc::OutboundPacketStream& p,
        const QString& header,
        const QString& statusTxtBody) {
    if (receiverIp) {
        UdpTransmitSocket transmitSocket(IpEndpointName(receiverIp, port));
        transmitSocket.Send(p.Data(), p.Size());
        qDebug() << QString("OSC Msg Send to Receiver (%1:%2) : <%3 : %4>")
                            .arg(receiverIp)
                            .arg(port)
                            .arg(header, statusTxtBody);
    }
}

void OscFunctionsSendPtrType(UserSettingsPointer m_pConfig,
        const QString& OscGroup,
        const QString& OscKey,
        enum DefOscBodyType OscBodyType,
        const QString& OscMessageBodyQString,
        int OscMessageBodyInt,
        double OscMessageBodyDouble,
        float OscMessageBodyFloat) {
    QString OscMessageHeader = "/" + OscGroup + "@" + OscKey;
    OscMessageHeader.replace("[", "(");
    OscMessageHeader.replace("]", ")");

    QByteArray OscMessageHeaderBa = OscMessageHeader.toLocal8Bit();
    const char* OscMessageHeaderChar = OscMessageHeaderBa.data();

    if (m_pConfig->getValue<bool>(ConfigKey("[OSC]", "OscEnabled"))) {
        char buffer[IP_MTU_SIZE];
        osc::OutboundPacketStream p(buffer, IP_MTU_SIZE);
        QString OscStatusTxtBody;

        // Creation of package
        p.Clear();
        p << osc::BeginBundle();
        switch (OscBodyType) {
        case DefOscBodyType::STRINGBODY:
            p << osc::BeginMessage(OscMessageHeaderChar)
              << OscMessageBodyQString.toLocal8Bit().data() << osc::EndMessage;
            OscStatusTxtBody = OscMessageBodyQString;
            break;
        case DefOscBodyType::INTBODY:
            p << osc::BeginMessage(OscMessageHeaderChar) << OscMessageBodyInt << osc::EndMessage;
            OscStatusTxtBody = QString::number(OscMessageBodyInt);
            break;
        case DefOscBodyType::DOUBLEBODY:
            p << osc::BeginMessage(OscMessageHeaderChar) << OscMessageBodyDouble << osc::EndMessage;
            OscStatusTxtBody = QString::number(OscMessageBodyDouble);
            break;
        case DefOscBodyType::FLOATBODY:
            p << osc::BeginMessage(OscMessageHeaderChar) << OscMessageBodyFloat << osc::EndMessage;
            OscStatusTxtBody = QString::number(OscMessageBodyFloat);
            break;
        }
        p << osc::EndBundle;

        // Retrieve output port
        int CKOscPortOutInt = m_pConfig->getValue(ConfigKey("[OSC]", "OscPortOut")).toInt();

        // List of similar parts of receiver
        const QList<std::pair<QString, QString>> receivers = {
                {"[OSC]", "OscReceiver1"},
                {"[OSC]", "OscReceiver2"},
                {"[OSC]", "OscReceiver3"},
                {"[OSC]", "OscReceiver4"},
                {"[OSC]", "OscReceiver5"}};

        // Send to active receivers
        for (const auto& receiver : receivers) {
            if (m_pConfig->getValue<bool>(ConfigKey(receiver.first, receiver.second + "Active"))) {
                QByteArray receiverIpBa =
                        m_pConfig
                                ->getValue(ConfigKey(
                                        receiver.first, receiver.second + "Ip"))
                                .toLocal8Bit();
                sendOscMessage(receiverIpBa.data(),
                        CKOscPortOutInt,
                        p,
                        OscMessageHeader,
                        OscStatusTxtBody);
            }
        }
    } else {
        qDebug() << "OSC NOT Enabled";
    }
}

void OscNoTrackLoadedInGroup(UserSettingsPointer m_pConfig, const QString& OscGroup) {
    OscFunctionsSendPtrType(m_pConfig,
            OscGroup,
            "TrackArtist",
            DefOscBodyType::STRINGBODY,
            "no track loaded",
            0,
            0,
            0);
    OscFunctionsSendPtrType(m_pConfig,
            OscGroup,
            "TrackTitle",
            DefOscBodyType::STRINGBODY,
            "no track loaded",
            0,
            0,
            0);
    OscFunctionsSendPtrType(m_pConfig,
            OscGroup,
            "duration",
            DefOscBodyType::FLOATBODY,
            "",
            0,
            0,
            0);
    OscFunctionsSendPtrType(m_pConfig,
            OscGroup,
            "track_loaded",
            DefOscBodyType::FLOATBODY,
            "",
            0,
            0,
            0);
    OscFunctionsSendPtrType(m_pConfig,
            OscGroup,
            "playposition",
            DefOscBodyType::FLOATBODY,
            "",
            0,
            0,
            0);
}

void OscTrackLoadedInGroup(UserSettingsPointer m_pConfig,
        const QString& OscGroup,
        const QString& TrackArtist,
        const QString& TrackTitle,
        float track_loaded,
        float duration,
        float playposition) {
    OscFunctionsSendPtrType(m_pConfig,
            OscGroup,
            "TrackArtist",
            DefOscBodyType::STRINGBODY,
            TrackArtist,
            0,
            0,
            0);
    OscFunctionsSendPtrType(m_pConfig,
            OscGroup,
            "TrackTitle",
            DefOscBodyType::STRINGBODY,
            TrackTitle,
            0,
            0,
            0);
    OscFunctionsSendPtrType(m_pConfig,
            OscGroup,
            "track_loaded",
            DefOscBodyType::FLOATBODY,
            "",
            0,
            0,
            track_loaded);
    OscFunctionsSendPtrType(m_pConfig,
            OscGroup,
            "duration",
            DefOscBodyType::FLOATBODY,
            "",
            0,
            0,
            duration);
    OscFunctionsSendPtrType(m_pConfig,
            OscGroup,
            "playposition",
            DefOscBodyType::FLOATBODY,
            "",
            0,
            0,
            playposition);
}

void OscChangedPlayState(UserSettingsPointer m_pConfig,
        const QString& OscGroup,
        float playstate) {
    OscFunctionsSendPtrType(m_pConfig,
            OscGroup,
            "play",
            DefOscBodyType::FLOATBODY,
            "",
            0,
            0,
            playstate);
}

#endif /* OSCFUNCTIONS_H */
