#ifndef OSCFUNCTIONS_H
#define OSCFUNCTIONS_H

constexpr int OUTPUT_BUFFER_SIZE = 1024;
constexpr int IP_MTU_SIZE = 1536;

#include <QChar>
#include <QDataStream>
#include <QFile>
#include <QRegularExpression>
#include <QSharedPointer>
#include <QString>
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

// function to convert and carry special characters in TrackArtist & TrackTitle to ASCII
QString escapeStringToJsonUnicode(const QString& input) {
    QString escaped;
    for (QChar c : input) {
        if (c.isPrint() && c.unicode() < 128) {
            // Keep printable ASCII characters as is
            escaped += c;
        } else {
            // Escape non-ASCII and special characters
            escaped += QString("\\u%1").arg(c.unicode(), 4, 16, QChar('0')).toUpper();
        }
    }
    return escaped;
}

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

void oscFunctionsSendPtrType(UserSettingsPointer pConfig,
        const QString& oscGroup,
        const QString& oscKey,
        enum DefOscBodyType oscBodyType,
        const QString& oscMessageBodyQString,
        int oscMessageBodyInt,
        double oscMessageBodyDouble,
        float oscMessageBodyFloat) {
    // QString oscMessageHeader = "/" + oscGroup + "@" + oscKey;
    QString oscMessageHeader = "/" + oscGroup + "/" + oscKey;
    // oscMessageHeader.replace("[", "(");
    oscMessageHeader.replace("[", "");
    // oscMessageHeader.replace("]", ")");
    oscMessageHeader.replace("]", "");

    QByteArray oscMessageHeaderBa = oscMessageHeader.toLocal8Bit();
    const char* oscMessageHeaderChar = oscMessageHeaderBa.data();

    if (pConfig->getValue<bool>(ConfigKey("[OSC]", "OscEnabled"))) {
        char buffer[IP_MTU_SIZE];
        osc::OutboundPacketStream p(buffer, IP_MTU_SIZE);
        QString oscStatusTxtBody;

        // Creation of package
        p.Clear();
        p << osc::BeginBundle();
        switch (oscBodyType) {
        case DefOscBodyType::STRINGBODY:
            p << osc::BeginMessage(oscMessageHeaderChar)
              << oscMessageBodyQString.toLocal8Bit().data() << osc::EndMessage;
            oscStatusTxtBody = oscMessageBodyQString;
            break;
        case DefOscBodyType::INTBODY:
            p << osc::BeginMessage(oscMessageHeaderChar) << oscMessageBodyInt << osc::EndMessage;
            oscStatusTxtBody = QString::number(oscMessageBodyInt);
            break;
        case DefOscBodyType::DOUBLEBODY:
            p << osc::BeginMessage(oscMessageHeaderChar) << oscMessageBodyDouble << osc::EndMessage;
            oscStatusTxtBody = QString::number(oscMessageBodyDouble);
            break;
        case DefOscBodyType::FLOATBODY:
            p << osc::BeginMessage(oscMessageHeaderChar) << oscMessageBodyFloat << osc::EndMessage;
            oscStatusTxtBody = QString::number(oscMessageBodyFloat);
            break;
        }
        p << osc::EndBundle;

        // Retrieve output port
        int ckOscPortOutInt = pConfig->getValue(ConfigKey("[OSC]", "OscPortOut")).toInt();

        // List of similar parts of receiver
        const QList<std::pair<QString, QString>> receivers = {
                {"[OSC]", "OscReceiver1"},
                {"[OSC]", "OscReceiver2"},
                {"[OSC]", "OscReceiver3"},
                {"[OSC]", "OscReceiver4"},
                {"[OSC]", "OscReceiver5"}};

        // Send to active receivers
        for (const auto& receiver : receivers) {
            if (pConfig->getValue<bool>(ConfigKey(receiver.first, receiver.second + "Active"))) {
                QByteArray receiverIpBa =
                        pConfig
                                ->getValue(ConfigKey(
                                        receiver.first, receiver.second + "Ip"))
                                .toLocal8Bit();
                sendOscMessage(receiverIpBa.data(),
                        ckOscPortOutInt,
                        p,
                        oscMessageHeader,
                        oscStatusTxtBody);
            }
        }
    } else {
        qDebug() << "OSC NOT Enabled";
    }
}

void sendNoTrackLoadedToOscClients(UserSettingsPointer pConfig, const QString& oscGroup) {
    oscFunctionsSendPtrType(pConfig,
            oscGroup,
            "TrackArtist",
            DefOscBodyType::STRINGBODY,
            "no track loaded",
            0,
            0,
            0);
    oscFunctionsSendPtrType(pConfig,
            oscGroup,
            "TrackTitle",
            DefOscBodyType::STRINGBODY,
            "no track loaded",
            0,
            0,
            0);
    oscFunctionsSendPtrType(pConfig,
            oscGroup,
            "duration",
            DefOscBodyType::FLOATBODY,
            "",
            0,
            0,
            0);
    oscFunctionsSendPtrType(pConfig,
            oscGroup,
            "track_loaded",
            DefOscBodyType::FLOATBODY,
            "",
            0,
            0,
            0);
    oscFunctionsSendPtrType(pConfig,
            oscGroup,
            "playposition",
            DefOscBodyType::FLOATBODY,
            "",
            0,
            0,
            0);
    QString oscKeyArtist = QString(oscGroup + "TrackArtist");
    QString oscKeyValueNoTrackLoaded = QString("no track loaded");
    pConfig->set(ConfigKey("[OSC]", oscKeyArtist), oscKeyValueNoTrackLoaded);
    QString oscKeyTitle = QString(oscGroup + "TrackTitle");
    pConfig->set(ConfigKey("[OSC]", oscKeyTitle), oscKeyValueNoTrackLoaded);
}

void sendTrackInfoToOscClients(UserSettingsPointer pConfig,
        const QString& oscGroup,
        const QString& trackArtist,
        const QString& trackTitle,
        float track_loaded,
        float duration,
        float playposition) {
    oscFunctionsSendPtrType(pConfig,
            oscGroup,
            "TrackArtist",
            DefOscBodyType::STRINGBODY,
            escapeStringToJsonUnicode(trackArtist),
            0,
            0,
            0);
    oscFunctionsSendPtrType(pConfig,
            oscGroup,
            "TrackTitle",
            DefOscBodyType::STRINGBODY,
            escapeStringToJsonUnicode(trackTitle),
            0,
            0,
            0);
    oscFunctionsSendPtrType(pConfig,
            oscGroup,
            "track_loaded",
            DefOscBodyType::FLOATBODY,
            "",
            0,
            0,
            track_loaded);
    oscFunctionsSendPtrType(pConfig,
            oscGroup,
            "duration",
            DefOscBodyType::FLOATBODY,
            "",
            0,
            0,
            duration);
    oscFunctionsSendPtrType(pConfig,
            oscGroup,
            "playposition",
            DefOscBodyType::FLOATBODY,
            "",
            0,
            0,
            playposition);
    QString oscKeyArtist = QString(oscGroup + "TrackArtist");
    pConfig->set(ConfigKey("[OSC]", oscKeyArtist), trackArtist);
    QString oscKeyTitle = QString(oscGroup + "TrackTitle");
    pConfig->set(ConfigKey("[OSC]", oscKeyTitle), trackTitle);
}

void oscChangedPlayState(UserSettingsPointer pConfig,
        const QString& oscGroup,
        float playstate) {
    oscFunctionsSendPtrType(pConfig,
            oscGroup,
            "play",
            DefOscBodyType::FLOATBODY,
            "",
            0,
            0,
            playstate);
}

// function to convert url-style /tree/branch/leaf to Mixxx-CO's
QString translatePath(const QString& inputPath) {
    const QString& originalPath = inputPath;

    // Static QRegularExpression objects to avoid recompilation
    static QRegularExpression stemSpecificRegex(R"(/([^/]+)/Channel(\d+)Stem(\d+)/(.*))");
    static QRegularExpression complexRackRegex(R"(/([^/]+)/([^/]+)/([^/]+)/([^/]+))");
    static QRegularExpression nestedRackRegex(R"(/([^/]+)/([^/]+)/([^/]+))");
    static QRegularExpression channelSpecificRegex(R"(/Channel(\d+)/(.*))");
    static QRegularExpression generalRegex(R"(/([^/]+)/([^/]+))");
    static QRegularExpression hotcueRegex(R"(/Channel(\d+)/hotcue/(activate|clear)/(\d+))");
    static QRegularExpression loopRegex(R"(/Channel(\d+)/(reloop|loop|beatloop|beatjump)/(.*))");
    static QRegularExpression autoDjRegex(R"(/AutoDJ/(.*))");
    static QRegularExpression libraryRegex(R"(/Library/(.*))");

    // Match paths with stems like /QuickEffectRack1/Channel2Stem1/super1
    QRegularExpressionMatch match = stemSpecificRegex.match(inputPath);
    if (match.hasMatch()) {
        QString rack = match.captured(1);      // e.g., "QuickEffectRack1"
        QString channel = match.captured(2);   // e.g., "2"
        QString stem = match.captured(3);      // e.g., "1"
        QString parameter = match.captured(4); // e.g., "super1"
        return QString("[%1_[Channel%2Stem%3]],%4").arg(rack, channel, stem, parameter);
    }

    // Match deeply nested paths like /EqualizerRack1/Channel2/Effect1/parameter1
    match = complexRackRegex.match(inputPath);
    if (match.hasMatch()) {
        QString rack = match.captured(1);          // e.g., "EqualizerRack1"
        QString channelOrUnit = match.captured(2); // e.g., "Channel2"
        QString effect = match.captured(3);        // e.g., "Effect1"
        QString parameter = match.captured(4);     // e.g., "parameter1"
        // Wrap the channelOrUnit in square brackets if it starts with "Channel"
        if (channelOrUnit.startsWith("Channel")) {
            channelOrUnit = "[" + channelOrUnit + "]";
        }
        return QString("[%1_%2_%3],%4").arg(rack, channelOrUnit, effect, parameter);
    }

    // Match nested paths like /EffectRack1/EffectUnit2/mix
    match = nestedRackRegex.match(inputPath);
    if (match.hasMatch()) {
        QString rack = match.captured(1);      // e.g., "EffectRack1"
        QString unit = match.captured(2);      // e.g., "EffectUnit2"
        QString parameter = match.captured(3); // e.g., "mix"
        return QString("[%1_%2],%3").arg(rack, unit, parameter);
    }

    // Handle specific Channel paths like /Channel1/track_loaded -> [Channel1],track_loaded
    match = channelSpecificRegex.match(inputPath);
    if (match.hasMatch()) {
        QString channel = match.captured(1); // e.g., "1"
        QString action = match.captured(2);  // e.g., "track_loaded"
        return QString("[Channel%1],%2").arg(channel, action);
    }

    // Handle general cases like /Channel2Stem3/mute -> [Channel2Stem3],mute
    match = generalRegex.match(inputPath);
    if (match.hasMatch()) {
        QString group = match.captured(1); // e.g., "Channel2Stem3"
        QString item = match.captured(2);  // e.g., "mute"
        return QString("[%1],%2").arg(group, item);
    }

    // Handle hotcue paths like /Channel1/hotcue/activate/2
    match = hotcueRegex.match(inputPath);
    if (match.hasMatch()) {
        QString channel = match.captured(1); // e.g., "1"
        QString action = match.captured(2);  // e.g., "activate"
        QString cue = match.captured(3);     // e.g., "2"
        return QString("[Channel%1],hotcue_%2_%3").arg(channel, cue, action);
    }

    // Handle loop paths like /Channel2/reloop/toggle
    match = loopRegex.match(inputPath);
    if (match.hasMatch()) {
        QString channel = match.captured(1); // e.g., "2"
        QString type = match.captured(2);    // e.g., "reloop"
        QString action = match.captured(3);  // e.g., "toggle"
        if (type == "beatjump" || type == "beatloop") {
            return QString("[Channel%1],%2_%3_%4").arg(channel, type, action, match.captured(3));
        }
        return QString("[Channel%1],%2_%3").arg(channel, type, action);
    }

    // Handle AutoDJ paths like /AutoDJ/fade
    match = autoDjRegex.match(inputPath);
    if (match.hasMatch()) {
        QString action = match.captured(1); // e.g., "fade"
        return QString("[AutoDJ],%1").arg(action);
    }

    // Handle Library paths like /Library/MoveRight
    match = libraryRegex.match(inputPath);
    if (match.hasMatch()) {
        QString action = match.captured(1); // e.g., "MoveRight"
        return QString("[Library],%1").arg(action);
    }

    // Return the input path unchanged if no matches are found
    qDebug() << "[OSC] OSCFUNCTIONS Original path:" << originalPath
             << " Translated path:" << inputPath;
    return inputPath;
}
#endif /* OSCFUNCTIONS_H */
