#include "analyzer/analyzerkey.h"

#include <QDir>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QString>
#include <QVector>
#include <QtDebug>

#include "analyzer/analyzertrack.h"
#include "analyzer/constants.h"
#if defined __KEYFINDER__
#include "analyzer/plugins/analyzerkeyfinder.h"
#endif
#include "analyzer/plugins/analyzerqueenmarykey.h"
#include "analyzer/plugins/analyzerqueenmarykeyextended.h"
#include "proto/keys.pb.h"
#include "track/keyfactory.h"
#include "track/track.h"

namespace {
constexpr int excludeFirstChannelMask = 0x1;
constexpr bool showDebugWAnalyzerKey = false;
} // namespace

// static
QList<mixxx::AnalyzerPluginInfo> AnalyzerKey::availablePlugins() {
    QList<mixxx::AnalyzerPluginInfo> analyzers;
    // First one below is the default
    // Extended analyzes to create "same key"-segments, for curves
    analyzers.push_back(mixxx::AnalyzerQueenMaryKeyExtended::pluginInfo());
    analyzers.push_back(mixxx::AnalyzerQueenMaryKey::pluginInfo());
#if defined __KEYFINDER__
    analyzers.push_back(mixxx::AnalyzerKeyFinder::pluginInfo());
#endif
    return analyzers;
}

// static
mixxx::AnalyzerPluginInfo AnalyzerKey::defaultPlugin() {
    const auto plugins = availablePlugins();
    DEBUG_ASSERT(!plugins.isEmpty());
    return plugins.at(0);
}

AnalyzerKey::AnalyzerKey(UserSettingsPointer pConfig)
        : m_pConfig(pConfig),
          m_keySettings(pConfig),
          m_sampleRate(0),
          m_totalFrames(0),
          m_maxFramesToProcess(0),
          m_currentFrame(0),
          m_bPreferencesKeyDetectionEnabled(true),
          m_bPreferencesFastAnalysisEnabled(false),
          m_bPreferencesReanalyzeEnabled(false) {
}

bool AnalyzerKey::initialize(const AnalyzerTrack& track,
        mixxx::audio::SampleRate sampleRate,
        mixxx::audio::ChannelCount channelCount,
        SINT frameLength) {
    if (frameLength <= 0) {
        return false;
    }

    m_bPreferencesKeyDetectionEnabled = m_keySettings.getKeyDetectionEnabled();
    if (!m_bPreferencesKeyDetectionEnabled) {
        qDebug() << "Key detection is deactivated";
        return false;
    }

    m_bPreferencesFastAnalysisEnabled = m_keySettings.getFastAnalysis();
    m_bPreferencesReanalyzeEnabled = m_keySettings.getReanalyzeWhenSettingsChange();

    const auto plugins = availablePlugins();
    if (!plugins.isEmpty()) {
        m_pluginId = defaultPlugin().id();
        QString pluginId = m_keySettings.getKeyPluginId();

        // Short tracks like samples are mostly to short to produce
        // meaningful key segments.
        // -> Fall back to the default Queen Mary plugin

        constexpr double kMinExtendedAnalysisSeconds = 10.0;

        const double trackDurationSeconds =
                static_cast<double>(frameLength) / sampleRate.toDouble();
        if (trackDurationSeconds < kMinExtendedAnalysisSeconds &&
                pluginId ==
                        mixxx::AnalyzerQueenMaryKeyExtended::pluginInfo().id()) {
            pluginId = mixxx::AnalyzerQueenMaryKey::pluginInfo().id();
            qDebug() << "[AnalyzerKey] Track is" << trackDurationSeconds
                     << "s - too short for extended key analysis, using"
                     << pluginId;
        }

        for (const auto& info : plugins) {
            if (info.id() == pluginId) {
                m_pluginId = pluginId; // configured Plug-In available
                break;
            }
        }
    }

    qDebug() << "AnalyzerKey preference settings:"
             << "\nPlugin:" << m_pluginId
             << "\nRe-analyze when settings change:" << m_bPreferencesReanalyzeEnabled
             << "\nFast analysis:" << m_bPreferencesFastAnalysisEnabled;

    m_sampleRate = sampleRate;
    m_channelCount = channelCount;
    m_totalFrames = frameLength;
    // In fast analysis mode, skip processing after
    // kFastAnalysisSecondsToAnalyze seconds are analyzed.
    if (m_bPreferencesFastAnalysisEnabled) {
        m_maxFramesToProcess = mixxx::kFastAnalysisSecondsToAnalyze * m_sampleRate;
    } else {
        m_maxFramesToProcess = frameLength;
    }
    m_currentFrame = 0;

    // if we can't load a stored track reanalyze it
    bool bShouldAnalyze = shouldAnalyze(track.getTrack());

    qDebug() << "[AnalyzerKey] initialize - Track:" << track.getTrack()->getTitle();
    qDebug() << "[AnalyzerKey]   Plugin ID: %s", m_pluginId.toUtf8().constData();

    DEBUG_ASSERT(!m_pPlugin);
    if (bShouldAnalyze) {
        if (m_pluginId == mixxx::AnalyzerQueenMaryKeyExtended::pluginInfo().id()) {
            m_pPlugin = std::make_unique<mixxx::AnalyzerQueenMaryKeyExtended>();
        } else if (m_pluginId == mixxx::AnalyzerQueenMaryKey::pluginInfo().id()) {
            m_pPlugin = std::make_unique<mixxx::AnalyzerQueenMaryKey>();
#if defined __KEYFINDER__
        } else if (m_pluginId == mixxx::AnalyzerKeyFinder::pluginInfo().id()) {
            m_pPlugin = std::make_unique<mixxx::AnalyzerKeyFinder>();
#endif
        } else {
            // This must not happen, because we have already verified above
            // that the PlugInId is valid
            DEBUG_ASSERT(false);
        }

        if (m_pPlugin) {
            if (m_pPlugin->initialize(mixxx::audio::SampleRate(m_sampleRate))) {
                qDebug() << "Key calculation started with plugin" << m_pluginId;
            } else {
                qDebug() << "Key calculation will not start.";
                m_pPlugin.reset();
                bShouldAnalyze = false;
            }
        } else {
            bShouldAnalyze = false;
        }
    }
    return bShouldAnalyze;
}

bool AnalyzerKey::shouldAnalyze(TrackPointer pTrack) const {
    bool bPreferencesFastAnalysisEnabled = m_keySettings.getFastAnalysis();
    QString pluginID = m_keySettings.getKeyPluginId();
    if (pluginID.isEmpty()) {
        pluginID = defaultPlugin().id();
    }

    // Check:
    // - If the QMExtended is selected -> to create the segments & curve
    // - Uf there aren't KEY-segments for the track in the track_key_segments table
    // ==> Execute the analysis to create the segments (and the curve)
    auto* pExtendedPlugin = dynamic_cast<mixxx::AnalyzerQueenMaryKeyExtended*>(m_pPlugin.get());
    if (pExtendedPlugin) {
        QList<KeySegmentsPointer> segments = pTrack->getKeySegments();
        if (segments.isEmpty()) {
            if (showDebugWAnalyzerKey) {
                qDebug() << "[AnalyzerBeats] - QM-Ext selected -> No KEY "
                            "segments in DB -> ANALYZE Track "
                         << pTrack->getTitle()
                         << "ID:" << pTrack->getId().toString();
            }
            return true;
        } else {
            if (showDebugWAnalyzerKey) {
                qDebug() << "[AnalyzerBeats] - QM-Ext selected -> KEY segments already in DB."
                         << "for track: " << pTrack->getTitle()
                         << "ID:" << pTrack->getId().toString();
            }
            if (m_bPreferencesReanalyzeEnabled) {
                if (showDebugWAnalyzerKey) {
                    qDebug() << "[AnalyzerBeats] - QM-Ext selected -> BPM segments already in DB"
                             << "User preferences: re-analyze old BPM segments"
                             << " -> ANALYZE Track " << pTrack->getTitle()
                             << "ID:" << pTrack->getId().toString();
                }
                return true;
            }
            return false;
        }
    }

    const Keys keys = pTrack->getKeys();

    qDebug() << "[AnalyzerKey] shouldAnalyze - Track:" << pTrack->getTitle();
    qDebug() << "[AnalyzerKey]   Current key:" << keys.getGlobalKey()
             << "Version:" << keys.getVersion()
             << "SubVersion:" << keys.getSubVersion();

    if (keys.getGlobalKey() != mixxx::track::io::key::INVALID) {
        QString version = keys.getVersion();
        QString subVersion = keys.getSubVersion();

        QHash<QString, QString> extraVersionInfo = getExtraVersionInfo(
                pluginID, bPreferencesFastAnalysisEnabled);
        QString newVersion = KeyFactory::getPreferredVersion();
        QString newSubVersion = KeyFactory::getPreferredSubVersion(extraVersionInfo);

        qDebug() << "[AnalyzerKey]   New version:" << newVersion
                 << "New subVersion:" << newSubVersion;
        qDebug() << "[AnalyzerKey]   Reanalyze enabled:" << m_bPreferencesReanalyzeEnabled;

        if (version == newVersion && subVersion == newSubVersion) {
            // If the version and settings have not changed then if the world is
            // sane, re-analyzing will do nothing.
            qDebug() << "[AnalyzerKey] -> SKIP -> Keys version/sub-version "
                        "unchanged since previous analysis. Not analyzing.";
            return false;
        }
        if (!m_bPreferencesReanalyzeEnabled) {
            qDebug() << "[AnalyzerKey] -> Track has previous key detection result that is not up"
                     << "to date with latest settings but user preferences"
                     << "indicate we should not re-analyze it.";
            return false;
        }
    }
    qDebug() << "[AnalyzerKey] -> WILL ANALYZE";
    return true;
}

bool AnalyzerKey::processSamples(const CSAMPLE* pIn, SINT count) {
    VERIFY_OR_DEBUG_ASSERT(m_pPlugin) {
        return false;
    }

    SINT numFrames = count / m_channelCount;
    m_currentFrame += numFrames;

    if (m_currentFrame > m_maxFramesToProcess) {
        return true; // silently ignore remaining samples
    }

    const CSAMPLE* pKeyInput = pIn;
    CSAMPLE* pHarmonicMixedChannel = nullptr;

    if (m_channelCount == mixxx::audio::ChannelCount::stem()) {
        // We have an 8 channel soundsource. The only implemented soundsource with
        // 8ch is the NI STEM file format.
        // TODO: If we add other soundsources with 8ch, we need to rework this condition.
        //
        // For NI STEM we mix all the stems together except the first one,
        // which contains drums or beats by convention.
        count = numFrames * mixxx::audio::ChannelCount::stereo();
        pHarmonicMixedChannel = SampleUtil::alloc(count);
        VERIFY_OR_DEBUG_ASSERT(pHarmonicMixedChannel) {
            return false;
        }

        if (m_keySettings.getStemStrategy() == KeyDetectionSettings::StemStrategy::Enforced) {
            SampleUtil::mixMultichannelToStereo(pHarmonicMixedChannel,
                    pIn,
                    numFrames,
                    m_channelCount,
                    excludeFirstChannelMask);
        } else {
            SampleUtil::mixMultichannelToStereo(
                    pHarmonicMixedChannel, pIn, numFrames, m_channelCount);
        }

        pKeyInput = pHarmonicMixedChannel;
    } else if (m_channelCount > mixxx::audio::ChannelCount::stereo()) {
        DEBUG_ASSERT(!"Unsupported channel count");
        return false;
    }

    bool ret = m_pPlugin->processSamples(pKeyInput, count);
    if (pHarmonicMixedChannel) {
        SampleUtil::free(pHarmonicMixedChannel);
    }
    return ret;
}

void AnalyzerKey::cleanup() {
    m_pPlugin.reset();
}

void AnalyzerKey::storeResults(TrackPointer tio) {
    VERIFY_OR_DEBUG_ASSERT(m_pPlugin) {
        return;
    }

    if (!m_pPlugin->finalize()) {
        qWarning() << "Key detection failed";
        return;
    }

    KeyChangeList key_changes = m_pPlugin->getKeyChanges();
    QHash<QString, QString> extraVersionInfo = getExtraVersionInfo(
            m_pluginId, m_bPreferencesFastAnalysisEnabled);
    Keys track_keys = KeyFactory::makePreferredKeys(
            key_changes, extraVersionInfo, m_sampleRate, m_totalFrames);
    tio->setKeys(track_keys);

    // KEY SEGMENTS -> JSON & DB
    auto* pExtendedPlugin = dynamic_cast<mixxx::AnalyzerQueenMaryKeyExtended*>(m_pPlugin.get());
    if (pExtendedPlugin) {
        if (showDebugWAnalyzerKey) {
            qDebug() << "[AnalyzerKey] ========================================";
            qDebug() << "[AnalyzerKey] Track:" << tio->getArtist() << "-" << tio->getTitle();
            qDebug() << "[AnalyzerKey] Duration:" << tio->getDuration() << "seconds";
            qDebug() << "[AnalyzerKey] ========================================";
        }
        QJsonArray segmentsArray = pExtendedPlugin->getKeySegmentsJson();

        // -> DB
        if (!segmentsArray.isEmpty()) {
            QList<KeySegmentsPointer> dbSegments;

            for (const QJsonValue& val : std::as_const(segmentsArray)) {
                if (!val.isObject()) {
                    continue;
                }

                QJsonObject obj = val.toObject();

                const int keyId = obj["keyId"].toInt();
                const QString keyText = obj["keyText"].toString();
                const double duration = obj["duration"].toDouble();
                if (keyId < 0 || keyId > 23 || keyText.isEmpty() || duration <= 0.0) {
                    qWarning() << "[AnalyzerKey] Skipping invalid key segment:"
                               << "keyId" << keyId
                               << "keyText" << keyText
                               << "duration" << duration;
                    continue;
                }

                KeySegmentsPointer pSegment(new KeySegments(
                        obj["position"].toDouble(),
                        duration,
                        keyId,
                        keyText,
                        obj["range_start"].toDouble(),
                        obj["range_end"].toDouble(),
                        obj["type"].toString(),
                        obj["confidence"].toDouble()));
                dbSegments.append(pSegment);
            }

            // mark segments dirty and save new segments
            tio->deleteKeySegments();
            if (tio->setKeySegments(dbSegments)) {
                if (showDebugWAnalyzerKey) {
                    qDebug() << "[AnalyzerKey] Saved" << dbSegments.size()
                             << "Key segments to track" << tio->getId();
                }
            }
        }
        if (showDebugWAnalyzerKey) {
            qDebug() << "[AnalyzerKey] Total segments:" << segmentsArray.size();
        }

        // -> JSON
        if (!segmentsArray.isEmpty()) {
            saveKeySegmentsJson(tio, segmentsArray);
        }
        if (showDebugWAnalyzerKey) {
            qDebug() << "[AnalyzerKey] ========================================";
            qDebug() << "[AnalyzerKey] Total segments:" << segmentsArray.size();
            qDebug() << "[AnalyzerKey] ========================================";
        }
    }
}

bool AnalyzerKey::saveKeySegmentsJson(TrackPointer pTrack, const QJsonArray& segmentsArray) {
    if (!pTrack) {
        if (showDebugWAnalyzerKey) {
            qDebug() << "[AnalyzerKey] No track to save key segments";
        }
        return false;
    }

    QString trackIdStr = pTrack->getId().toString();
    if (trackIdStr.isEmpty()) {
        if (showDebugWAnalyzerKey) {
            qDebug() << "[AnalyzerKey] No valid track ID";
        }
        return false;
    }

    QJsonObject root;
    root["key_curve"] = segmentsArray;

    QJsonObject trackInfo;
    trackInfo["title"] = pTrack->getTitle();
    trackInfo["artist"] = pTrack->getArtist();
    trackInfo["album"] = pTrack->getAlbum();
    trackInfo["duration_seconds"] = pTrack->getDuration();
    trackInfo["offset_seconds"] = 0.0;
    trackInfo["track_id"] = trackIdStr;

    root["track"] = trackInfo;

    QString keyCurvePath = m_pConfig->getSettingsPath() + "/keycurve/";
    QDir saveLocation;
    if (!saveLocation.mkpath(keyCurvePath)) {
        if (showDebugWAnalyzerKey) {
            qDebug() << "[AnalyzerKey] Failed to create directory:" << keyCurvePath;
        }
    }
    QString keyCurveFileLocation = keyCurvePath + pTrack->getId().toString() + ".json";

    QFile file(keyCurveFileLocation);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
        file.close();
        if (showDebugWAnalyzerKey) {
            qDebug() << "[AnalyzerKey] Saved key curve JSON to:" << keyCurveFileLocation;
            qDebug() << "[AnalyzerKey] Segments:" << segmentsArray.size();
        }
        return true;
    } else {
        if (showDebugWAnalyzerKey) {
            qDebug() << "[AnalyzerKey] Failed to save key curve JSON to:" << keyCurveFileLocation;
        }
        return false;
    }
}

// static
QHash<QString, QString> AnalyzerKey::getExtraVersionInfo(
        const QString& pluginId, bool bPreferencesFastAnalysis) {
    QHash<QString, QString> extraVersionInfo;
    extraVersionInfo["vamp_plugin_id"] = pluginId;
    if (bPreferencesFastAnalysis) {
        extraVersionInfo["fast_analysis"] = "1";
    }
    return extraVersionInfo;
}
