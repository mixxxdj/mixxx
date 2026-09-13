#include "oscfunctions.h"

#include <QDebug>
#include <QMutexLocker>
#include <QReadLocker>

QMutex s_configMutex;
QReadWriteLock g_oscTrackTableLock;
std::atomic<bool> s_oscEnabled(false);
int s_ckOscPortOutInt = 0;
QList<std::pair<bool, QString>> s_receiverConfigs;
std::atomic<bool> s_configLoaded1stTimeFromFile(false);
std::array<std::tuple<QString, QString, QString>, kMaxOscTracks> g_oscTrackTable;
QMutex g_oscTrackTableMutex;
bool s_oscSendSyncTriggers = false;
int s_oscSendSyncTriggersInterval = 0;
int s_lastCheckStamp = 0;
std::atomic<qint64> s_lastTriggerTime = 0;

OscFunctions::OscFunctions(UserSettingsPointer pConfig)
        : m_pConfig(pConfig) {
}

// function to convert and carry special characters in TrackArtist & TrackTitle to ASCII
QString OscFunctions::escapeStringToJsonUnicode(const QString& input) {
    QString escaped;
    escaped.reserve(input.size());

    for (QChar c : input) {
        if (c.isPrint() && c.unicode() < 128) {
            escaped += c;
        } else {
            escaped += QStringLiteral("\\u%1")
                               .arg(static_cast<uint>(c.unicode()),
                                       4,
                                       16,
                                       QLatin1Char('0'))
                               .toUpper();
        }
    }
    return escaped;
}

// Function to send OSC message with liblo
void OscFunctions::oscFunctionsSendPtrType(
        const QString& oscGroup,
        const QString& oscKey,
        enum DefOscBodyType oscBodyType,
        const QString& oscMessageBodyQString,
        int oscMessageBodyInt,
        double oscMessageBodyDouble,
        float oscMessageBodyFloat) {
    QString oscMessageHeader = "/" + oscGroup + "/" + oscKey;
    oscMessageHeader.replace("[", "");
    oscMessageHeader.replace("]", "");

    if (sDebugOSCFunctions) {
        qDebug() << "[OSC] [OSCFUNCTIONS] -> oscFunctionsSendPtrType -> start";
    }

    if (s_oscEnabled.load()) {
        for (const auto& receiver : std::as_const(s_receiverConfigs)) {
            if (receiver.first) {
                QByteArray receiverIpBa = receiver.second.toLocal8Bit();

                lo_address address = lo_address_new_with_proto(
                        LO_UDP,
                        receiverIpBa.data(),
                        std::to_string(s_ckOscPortOutInt).c_str());

                lo_message msg = lo_message_new();
                int result = -1;
                QString oscStatusTxtBody;

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
                } else if (sDebugOSCFunctions) {
                    qDebug()
                            << QString("[OSC] [OSCFUNCTIONS] -> Msg Send to "
                                       "Receiver (%1:%2) : <%3 : %4>")
                                       .arg(receiverIpBa.data(),
                                               QString::number(s_ckOscPortOutInt),
                                               oscMessageHeader,
                                               oscStatusTxtBody);
                }

                lo_message_free(msg);
                lo_address_free(address);
            }
        }
    } else if (sDebugOSCFunctions) {
        qDebug() << "[OSC] [OSCFUNCTIONS] -> OSC NOT Enabled";
    }
}

// function to reload the config OSC settings -> maybe call if they changed
void OscFunctions::reloadOscConfiguration() {
    QMutexLocker locker(&s_configMutex);

    if (!m_pConfig) {
        qWarning() << "[OSC] [OSCFUNCTIONS] -> pConfig is nullptr! Aborting reload.";
        return;
    }

    s_oscEnabled = m_pConfig->getValue<bool>(ConfigKey("[OSC]", "OscEnabled"));
    s_ckOscPortOutInt = m_pConfig->getValue(ConfigKey("[OSC]", "OscPortOut")).toInt();

    s_receiverConfigs.clear();

    const QList<std::pair<QString, QString>> receivers = {
            {"[OSC]", "OscReceiver1"},
            {"[OSC]", "OscReceiver2"},
            {"[OSC]", "OscReceiver3"},
            {"[OSC]", "OscReceiver4"},
            {"[OSC]", "OscReceiver5"}};

    for (const auto& receiver : receivers) {
        bool active = m_pConfig->getValue<bool>(
                ConfigKey(receiver.first, receiver.second + "Active"));
        QString ip = m_pConfig->getValue(
                ConfigKey(receiver.first, receiver.second + "Ip"));
        s_receiverConfigs.append({active, ip});
    }

    if (sDebugOSCFunctions) {
        qDebug() << "[OSC] [OSCFUNCTIONS] -> OSC configuration reloaded.";
    }
}

void OscFunctions::storeTrackInfo(const QString& oscGroup,
        const QString& trackArtist,
        const QString& trackTitle) {
    QMutexLocker locker(&g_oscTrackTableMutex);

    // No existing oscGroup entry? -> update
    for (auto& entry : g_oscTrackTable) {
        if (std::get<0>(entry) == oscGroup) {
            std::get<1>(entry) = trackArtist;
            std::get<2>(entry) = trackTitle;
            if (sDebugOSCFunctions) {
                qDebug() << "[OSC] [OSCFUNCTIONS] -> Updated Track Info: "
                         << oscGroup << trackArtist << trackTitle;
            }
            return;
        }
    }

    // No existing entry? -> New entry
    for (int i = 0; i < kMaxOscTracks; ++i) {
        if (std::get<0>(g_oscTrackTable[i]).isEmpty()) { // Find empty slot
            g_oscTrackTable[i] = std::make_tuple(oscGroup, trackArtist, trackTitle);
            if (sDebugOSCFunctions) {
                qDebug() << "[OSC] [OSCFUNCTIONS] -> Stored New Track Info: "
                         << oscGroup << trackArtist << trackTitle;
            }
            return;
        }
    }

    if (sDebugOSCFunctions) {
        qDebug() << "[OSC] -> Track Table is FULL! Cannot store more.";
    }
}

QString OscFunctions::getTrackInfo(const QString& oscGroup, const QString& oscKey) {
    QReadLocker locker(&g_oscTrackTableLock);
    for (const auto& entry : g_oscTrackTable) {
        if (std::get<0>(entry) == oscGroup) {
            if (oscKey == "TrackArtist") {
                return std::get<1>(entry);
            } else if (oscKey == "TrackTitle") {
                return std::get<2>(entry);
            }
        }
    }
    return "Unknown";
}

void OscFunctions::sendNoTrackLoadedToOscClients(const QString& oscGroup) {
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
    const QString& trackArtist = "no track loaded";
    const QString& trackTitle = "no track loaded";
    storeTrackInfo(oscGroup, trackArtist, trackTitle);
}

void OscFunctions::sendTrackInfoToOscClients(
        const QString& oscGroup,
        const QString& trackArtist,
        const QString& trackTitle,
        const QString& trackLocation,
        const QString& trackAlbum,
        float trackBPM,
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
            "TrackLocation",
            DefOscBodyType::STRINGBODY,
            escapeStringToJsonUnicode(trackLocation),
            0,
            0,
            0);
    oscFunctionsSendPtrType(
            oscGroup,
            "TrackAlbum",
            DefOscBodyType::STRINGBODY,
            escapeStringToJsonUnicode(trackAlbum),
            0,
            0,
            0);
    oscFunctionsSendPtrType(
            oscGroup,
            "TrackBMM",
            DefOscBodyType::FLOATBODY,
            "",
            0,
            0,
            trackBPM);
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
    storeTrackInfo(oscGroup, trackArtist, trackTitle);
}

void OscFunctions::oscChangedPlayState(
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
QString OscFunctions::translatePath(const QString& inputPath) {
    const QString& originalPath = inputPath;

    // Static QRegularExpression objects to avoid recompilation
    static QRegularExpression stemSpecificRegex(R"(/([^/]+)/Channel(\d+)Stem(\d+)/(.*))");
    // stem fx
    static QRegularExpression underscoreStemRegex(R"(/([^/]+)/Channel(\d+)_Stem(\d+)/(.*))");
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

    // Match paths with underscore stems like /QuickEffectRack1/Channel1_Stem1/enabled
    QRegularExpressionMatch underscoreMatch = underscoreStemRegex.match(inputPath);
    if (underscoreMatch.hasMatch()) {
        QString rack = underscoreMatch.captured(1);      // e.g., "QuickEffectRack1"
        QString channel = underscoreMatch.captured(2);   // e.g., "1"
        QString stem = underscoreMatch.captured(3);      // e.g., "1"
        QString parameter = underscoreMatch.captured(4); // e.g., "enabled"
        return QString("[%1_[Channel%2_Stem%3]],%4").arg(rack, channel, stem, parameter);
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
    if (sDebugOSCFunctions) {
        qDebug() << "[OSC] [OSCFUNCTIONS] -> Original path:" << originalPath
                 << " Translated path:" << inputPath;
    }
    return inputPath;
}
