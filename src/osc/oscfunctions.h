#ifndef OSCFUNCTIONS_H
#define OSCFUNCTIONS_H

#include <QChar>
#include <QDataStream>
#include <QDebug>
#include <QFile>
#include <QMutex>
#include <QRegularExpression>
#include <QSharedPointer>
#include <QString>
#include <atomic>
#include <iostream>
#include <memory>

#include "lo/lo.h"
static QMutex s_configMutex;
QReadWriteLock g_oscTrackTableLock;
extern std::atomic<bool> s_oscEnabled;
extern int s_ckOscPortOutInt;
extern QList<std::pair<bool, QString>> s_receiverConfigs;
extern std::atomic<bool> s_configLoaded1stTimeFromFile;

constexpr int kMaxOscTracks = 70;
inline std::array<std::tuple<QString, QString, QString>, kMaxOscTracks> g_oscTrackTable;
inline QMutex g_oscTrackTableMutex;

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

// Function to send OSC message with liblo
// void sendOscMessage(const char* receiverIp,
//        int port,
//        const QString& oscMessageHeader,
//        const char* bodyType,
//        QVariant oscMessageBodyData) {
//}

void oscFunctionsSendPtrType(
        // UserSettingsPointer pConfig,
        const QString& oscGroup,
        const QString& oscKey,
        enum DefOscBodyType oscBodyType,
        const QString& oscMessageBodyQString,
        int oscMessageBodyInt,
        double oscMessageBodyDouble,
        float oscMessageBodyFloat) {

    // Proceed with sending OSC messages using the stored values
    QString oscMessageHeader = "/" + oscGroup + "/" + oscKey;
    oscMessageHeader.replace("[", "");
    oscMessageHeader.replace("]", "");
    qDebug() << "[OSC] [OSCFUNCTIONS] -> oscFunctionsSendPtrType -> start";

    if (s_oscEnabled.load()) {
        for (const auto& receiver : s_receiverConfigs) {
            if (receiver.first) { // Check if the receiver is active
                QByteArray receiverIpBa = receiver.second.toLocal8Bit();

                // Create a new OSC message
                lo_address address = lo_address_new_with_proto(LO_UDP,
                        receiverIpBa.data(),
                        std::to_string(s_ckOscPortOutInt).c_str());
                lo_message msg = lo_message_new();
                int result = -1;
                QString oscStatusTxtBody;

                // Prepare the message body
                switch (oscBodyType) {
                case DefOscBodyType::STRINGBODY:
                    oscStatusTxtBody = oscMessageBodyQString;
                    result = lo_send(address,
                            oscMessageHeader.toLocal8Bit().data(),
                            "s",
                            oscMessageBodyQString.toLocal8Bit().data());
                    break;
                case DefOscBodyType::INTBODY:
                    oscStatusTxtBody = QString::number(oscMessageBodyInt);
                    result = lo_send(address,
                            oscMessageHeader.toLocal8Bit().data(),
                            "i",
                            oscMessageBodyInt);
                    break;
                case DefOscBodyType::DOUBLEBODY:
                    oscStatusTxtBody = QString::number(oscMessageBodyDouble);
                    result = lo_send(address,
                            oscMessageHeader.toLocal8Bit().data(),
                            "d",
                            oscMessageBodyDouble);
                    break;
                case DefOscBodyType::FLOATBODY:
                    oscStatusTxtBody = QString::number(oscMessageBodyFloat);
                    result = lo_send(address,
                            oscMessageHeader.toLocal8Bit().data(),
                            "f",
                            oscMessageBodyFloat);
                    break;
                }

                if (result == -1) {
                    qWarning() << "[OSC] [OSCFUNCTIONS] -> Error sending OSC message.";
                } else {
                    qDebug()
                            << QString("[OSC] [OSCFUNCTIONS] -> Msg Send to "
                                       "Receiver (%1:%2) : <%3 : %4>")
                                       .arg(receiverIpBa.data())
                                       .arg(s_ckOscPortOutInt)
                                       .arg(oscMessageHeader)
                                       .arg(oscStatusTxtBody);
                }

                lo_message_free(msg);
                lo_address_free(address);
            }
        }
    } else {
        qDebug() << "[OSC] [OSCFUNCTIONS] -> OSC NOT Enabled";
    }
}

// function to reload the config OSC settinfs -> maybe call if they changed
// not used at the moment
void reloadOscConfiguration(UserSettingsPointer pConfig) {
    QMutexLocker locker(&s_configMutex);

    if (!pConfig) {
        qWarning() << "[OSC] [OSCFUNCTIONS] -> pConfig is nullptr! Aborting reload.";
        return;
    }

    s_oscEnabled = pConfig->getValue<bool>(ConfigKey("[OSC]", "OscEnabled"));
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

    qDebug() << "[OSC] [OSCFUNCTIONS] -> OSC configuration reloaded.";
}

void storeTrackInfo(const QString& oscGroup,
        const QString& trackArtist,
        const QString& trackTitle) {
    QMutexLocker locker(&g_oscTrackTableMutex); // Lock for thread safety

    // No existing oscGroup entry? -> update
    for (auto& entry : g_oscTrackTable) {
        if (std::get<0>(entry) == oscGroup) {
            std::get<1>(entry) = trackArtist;
            std::get<2>(entry) = trackTitle;
            qDebug() << "[OSC] [OSCFUNCTIONS] -> Updated Track Info: "
                     << oscGroup << trackArtist << trackTitle;
            return; // Exit function after updating
        }
    }

    // No existing entry? -> New entry
    for (int i = 0; i < kMaxOscTracks; ++i) {
        if (std::get<0>(g_oscTrackTable[i]).isEmpty()) { // Find empty slot
            g_oscTrackTable[i] = std::make_tuple(oscGroup, trackArtist, trackTitle);
            qDebug() << "[OSC] [OSCFUNCTIONS] -> Stored New Track Info: "
                     << oscGroup << trackArtist << trackTitle;
            return;
        }
    }

    qDebug() << "[OSC] -> Track Table is FULL! Cannot store more.";
}

QString getTrackInfo(const QString& oscGroup, const QString& oscKey) {
    // QMutexLocker locker(&g_oscTrackTableMutex); // Lock for thread safety
    QReadLocker locker(&g_oscTrackTableLock); // Read lock
    for (const auto& entry : g_oscTrackTable) {
        if (std::get<0>(entry) == oscGroup) {
            if (oscKey == "TrackArtist") {
                return std::get<1>(entry);
            } else if (oscKey == "TrackTitle") {
                return std::get<2>(entry);
            }
        }
    }
    return "Unknown"; // Default value if not found
}

void sendNoTrackLoadedToOscClients(const QString& oscGroup) {
    oscFunctionsSendPtrType(
            oscGroup,
            "TrackArtist",
            DefOscBodyType::STRINGBODY,
            "no track loaded",
            0,
            0,
            0);
    oscFunctionsSendPtrType(
            oscGroup,
            "TrackTitle",
            DefOscBodyType::STRINGBODY,
            "no track loaded",
            0,
            0,
            0);
    oscFunctionsSendPtrType(
            oscGroup,
            "duration",
            DefOscBodyType::FLOATBODY,
            "",
            0,
            0,
            0);
    oscFunctionsSendPtrType(
            oscGroup,
            "track_loaded",
            DefOscBodyType::FLOATBODY,
            "",
            0,
            0,
            0);
    oscFunctionsSendPtrType(
            oscGroup,
            "playposition",
            DefOscBodyType::FLOATBODY,
            "",
            0,
            0,
            0);
    //    QString oscKeyArtist = QString(oscGroup + "TrackArtist");
    //    QString oscKeyValueNoTrackLoaded = QString("no track loaded");
    //    pConfig->set(ConfigKey("[OSC]", oscKeyArtist), oscKeyValueNoTrackLoaded);
    //    QString oscKeyTitle = QString(oscGroup + "TrackTitle");
    //    pConfig->set(ConfigKey("[OSC]", oscKeyTitle), oscKeyValueNoTrackLoaded);
    const QString& trackArtist = "no track loaded";
    const QString& trackTitle = "no track loaded";
    storeTrackInfo(oscGroup, trackArtist, trackTitle);
}

void sendTrackInfoToOscClients(
        const QString& oscGroup,
        const QString& trackArtist,
        const QString& trackTitle,
        float track_loaded,
        float duration,
        float playposition) {
    oscFunctionsSendPtrType(
            oscGroup,
            "TrackArtist",
            DefOscBodyType::STRINGBODY,
            escapeStringToJsonUnicode(trackArtist),
            0,
            0,
            0);
    oscFunctionsSendPtrType(
            oscGroup,
            "TrackTitle",
            DefOscBodyType::STRINGBODY,
            escapeStringToJsonUnicode(trackTitle),
            0,
            0,
            0);
    oscFunctionsSendPtrType(
            oscGroup,
            "track_loaded",
            DefOscBodyType::FLOATBODY,
            "",
            0,
            0,
            track_loaded);
    oscFunctionsSendPtrType(
            oscGroup,
            "duration",
            DefOscBodyType::FLOATBODY,
            "",
            0,
            0,
            duration);
    oscFunctionsSendPtrType(
            oscGroup,
            "playposition",
            DefOscBodyType::FLOATBODY,
            "",
            0,
            0,
            playposition);
    //    QString oscKeyArtist = QString(oscGroup + "TrackArtist");
    //    pConfig->set(ConfigKey("[OSC]", oscKeyArtist), trackArtist);
    //    QString oscKeyTitle = QString(oscGroup + "TrackTitle");
    //    pConfig->set(ConfigKey("[OSC]", oscKeyTitle), trackTitle);
    storeTrackInfo(oscGroup, trackArtist, trackTitle);
}

void oscChangedPlayState(
        const QString& oscGroup,
        float playstate) {
    oscFunctionsSendPtrType(
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
    // if (sDebug) {
    qDebug() << "[OSC] [OSCFUNCTIONS] -> Original path:" << originalPath
             << " Translated path:" << inputPath;
    // }
    return inputPath;
}
#endif /* OSCFUNCTIONS_H */
