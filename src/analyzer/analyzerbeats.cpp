#include "analyzer/analyzerbeats.h"

#include <QDir>
#include <QFile>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QVector>
#include <QtDebug>

#include "analyzer/analyzertrack.h"
#include "analyzer/constants.h"
#include "analyzer/plugins/analyzerqueenmarybeats.h"
#include "analyzer/plugins/analyzerqueenmarybeatsextended.h"
#include "analyzer/plugins/analyzersoundtouchbeats.h"
#include "library/rekordbox/rekordboxconstants.h"
#include "track/beatfactory.h"
#include "track/bpmsegments.h"
#include "track/track.h"

constexpr bool showDebugWAnalyzerBeats = false;
// static
QList<mixxx::AnalyzerPluginInfo> AnalyzerBeats::availablePlugins() {
    QList<mixxx::AnalyzerPluginInfo> plugins;
    // First one below is the default
    // QMextended = analyzing and defining segments for curves
    plugins.append(mixxx::AnalyzerQueenMaryBeatsExtended::pluginInfo());
    plugins.append(mixxx::AnalyzerQueenMaryBeats::pluginInfo());
    plugins.append(mixxx::AnalyzerSoundTouchBeats::pluginInfo());
    return plugins;
}

// static
mixxx::AnalyzerPluginInfo AnalyzerBeats::defaultPlugin() {
    const auto plugins = availablePlugins();
    DEBUG_ASSERT(!plugins.isEmpty());
    return plugins.at(0);
}

AnalyzerBeats::AnalyzerBeats(UserSettingsPointer pConfig, bool enforceBpmDetection)
        : m_bpmSettings(pConfig),
          m_enforceBpmDetection(enforceBpmDetection),
          m_bPreferencesReanalyzeOldBpm(false),
          m_bPreferencesReanalyzeImported(false),
          m_bPreferencesFixedTempo(true),
          m_bPreferencesFastAnalysis(false),
          m_maxFramesToProcess(0),
          m_currentFrame(0),
          m_pConfig(pConfig) {
}

bool AnalyzerBeats::initialize(const AnalyzerTrack& track,
        mixxx::audio::SampleRate sampleRate,
        mixxx::audio::ChannelCount channelCount,
        SINT frameLength) {
    if (frameLength <= 0) {
        return false;
    }

    bool bPreferencesBeatDetectionEnabled =
            m_enforceBpmDetection || m_bpmSettings.getBpmDetectionEnabled();
    if (!bPreferencesBeatDetectionEnabled) {
        qDebug() << "Beat calculation is deactivated";
        return false;
    }

    bool bpmLock = track.getTrack()->isBpmLocked();
    if (bpmLock) {
        qDebug() << "Track is BpmLocked: Beat calculation will not start";
        return false;
    }

    m_bPreferencesFixedTempo = track.getOptions().useFixedTempo.value_or(
            m_bpmSettings.getFixedTempoAssumption());
    m_bPreferencesReanalyzeOldBpm = m_bpmSettings.getReanalyzeWhenSettingsChange();
    m_bPreferencesReanalyzeImported = m_bpmSettings.getReanalyzeImported();
    m_bPreferencesFastAnalysis = m_bpmSettings.getFastAnalysis();

    const auto plugins = availablePlugins();
    if (!plugins.isEmpty()) {
        m_pluginId = defaultPlugin().id();
        QString pluginId = m_bpmSettings.getBeatPluginId();
        // Short tracks like samples don't produce meaningful BPM
        // -> Fall back to the default Queen Mary plugin for them.

        constexpr double kMinExtendedAnalysisSeconds = 10.0;

        const double trackDurationSeconds =
                static_cast<double>(frameLength) / sampleRate.toDouble();
        if (trackDurationSeconds < kMinExtendedAnalysisSeconds &&
                pluginId ==
                        mixxx::AnalyzerQueenMaryBeatsExtended::pluginInfo().id()) {
            pluginId = mixxx::AnalyzerQueenMaryBeats::pluginInfo().id();
            qDebug() << "[AnalyzerBeats] Track is" << trackDurationSeconds
                     << "s - too short for extended BPM analysis, using"
                     << pluginId;
        }

        for (const auto& info : plugins) {
            if (info.id() == pluginId) {
                m_pluginId = pluginId; // configured Plug-In available
                break;
            }
        }
    }

    qDebug() << "AnalyzerBeats preference settings:"
             << "\nPlugin:" << m_pluginId
             << "\nFixed tempo assumption:" << m_bPreferencesFixedTempo
             << "\nRe-analyze when settings change:" << m_bPreferencesReanalyzeOldBpm
             << "\nRe-analyze imported from other software:" << m_bPreferencesReanalyzeImported
             << "\nFast analysis:" << m_bPreferencesFastAnalysis;

    m_sampleRate = sampleRate;
    m_channelCount = channelCount;
    // In fast analysis mode, skip processing after
    // kFastAnalysisSecondsToAnalyze seconds are analyzed.
    if (m_bPreferencesFastAnalysis) {
        m_maxFramesToProcess =
                mixxx::kFastAnalysisSecondsToAnalyze * m_sampleRate;
    } else {
        m_maxFramesToProcess = frameLength;
    }
    m_currentFrame = 0;

    // if we can load a stored track don't reanalyze it
    bool bShouldAnalyze = shouldAnalyze(track.getTrack());

    DEBUG_ASSERT(!m_pPlugin);
    if (bShouldAnalyze) {
        // Eve for BPM segments -> extended QM
        if (m_pluginId == mixxx::AnalyzerQueenMaryBeatsExtended::pluginInfo().id()) {
            m_pPlugin = std::make_unique<mixxx::AnalyzerQueenMaryBeatsExtended>();
        } else if (m_pluginId == mixxx::AnalyzerQueenMaryBeats::pluginInfo().id()) {
            m_pPlugin = std::make_unique<mixxx::AnalyzerQueenMaryBeats>();
        } else if (m_pluginId == mixxx::AnalyzerSoundTouchBeats::pluginInfo().id()) {
            m_pPlugin = std::make_unique<mixxx::AnalyzerSoundTouchBeats>();
        } else {
            // This must not happen, because we have already verified above
            // that the PlugInId is valid
            DEBUG_ASSERT(false);
        }

        if (m_pPlugin) {
            if (m_pPlugin->initialize(m_sampleRate)) {
                qDebug() << "Beat calculation started with plugin" << m_pluginId;
            } else {
                qDebug() << "Beat calculation will not start.";
                m_pPlugin.reset();
                bShouldAnalyze = false;
            }
        } else {
            bShouldAnalyze = false;
        }
    }
    return bShouldAnalyze;
}

bool AnalyzerBeats::shouldAnalyze(TrackPointer pTrack) const {
    bool bpmLock = pTrack->isBpmLocked();
    if (bpmLock) {
        qDebug() << "Track is BpmLocked: Beat calculation will not start";
        return false;
    }

    QString pluginID = m_bpmSettings.getBeatPluginId();
    if (pluginID.isEmpty()) {
        pluginID = defaultPlugin().id();
    }

    // Check:
    // - If the QMExtended is selected -> to create the segments & curve
    // - Uf there aren't BPM-segments for the track in the track_bpm_segments
    // table
    // ==> Execute the analysis to create the segments (and the curve)

    bool isExtendedPlugin = (pluginID == "qm-tempotracker-extended:0");

    if (isExtendedPlugin) {
        QList<BpmSegmentsPointer> segments = pTrack->getBpmSegments();
        if (segments.isEmpty()) {
            if (showDebugWAnalyzerBeats) {
                qDebug() << "[AnalyzerBeats] - QM-Ext selected -> No BPM "
                            "segments in DB -> ANALYZE Track "
                         << pTrack->getTitle()
                         << "ID:" << pTrack->getId().toString();
            }
            return true;
        } else {
            if (showDebugWAnalyzerBeats) {
                qDebug() << "[AnalyzerBeats] - QM-Ext selected -> BPM segments already in DB."
                         << "for track: " << pTrack->getTitle()
                         << "ID:" << pTrack->getId().toString();
            }

            if (m_bPreferencesReanalyzeOldBpm) {
                if (showDebugWAnalyzerBeats) {
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

    // If the track already has a Beats object then we need to decide whether to
    // analyze this track or not.
    const mixxx::BeatsPointer pBeats = pTrack->getBeats();
    if (!pBeats) {
        return true;
    }
    if (!pBeats->getBpmInRange(mixxx::audio::kStartFramePos,
                       mixxx::audio::FramePos{
                               pTrack->getDuration() * pBeats->getSampleRate()})
                    .isValid()) {
        // Tracks with an invalid bpm <= 0 should be re-analyzed,
        // independent of the preference settings. We expect that
        // all tracks have a bpm > 0 when analyzed. Users that want
        // to keep their zero bpm tracks could lock them to prevent
        // this re-analysis (see the check above).
        qDebug() << "Re-analyzing track with invalid BPM despite preference settings.";
        return true;
    }

    QString subVersion = pBeats->getSubVersion();
    if (subVersion == mixxx::rekordboxconstants::beatsSubversion) {
        return m_bPreferencesReanalyzeImported;
    }

    if (subVersion.isEmpty() && pBeats->firstBeat() <= mixxx::audio::kStartFramePos &&
            m_pluginId != mixxx::AnalyzerSoundTouchBeats::pluginInfo().id()) {
        // This happens if the beat grid was created from the metadata BPM value.
        qDebug() << "First beat is 0 for grid so analyzing track to find first beat.";
        return true;
    }

    QString version = pBeats->getVersion();
    QHash<QString, QString> extraVersionInfo = getExtraVersionInfo(
            pluginID,
            m_bPreferencesFastAnalysis);
    QString newVersion = BeatFactory::getPreferredVersion(
            m_bPreferencesFixedTempo);
    QString newSubVersion = BeatFactory::getPreferredSubVersion(
            extraVersionInfo);

    if (version == newVersion && subVersion == newSubVersion) {
        // If the version and settings have not changed then if the world is
        // sane, re-analyzing will do nothing.
        return false;
    }
    // Beat grid exists but version and settings differ
    if (!m_bPreferencesReanalyzeOldBpm) {
        qDebug() << "Beat calculation skips analyzing because the track has"
                 << "a BPM computed by a previous Mixxx version and user"
                 << "preferences indicate we should not change it."
                 << "Track: " << pTrack->getTitle()
                 << "ID:" << pTrack->getId().toString();
        return false;
    }

    return true;
}

bool AnalyzerBeats::processSamples(const CSAMPLE* pIn, SINT count) {
    VERIFY_OR_DEBUG_ASSERT(m_pPlugin) {
        return false;
    }

    SINT numFrames = count / m_channelCount;
    const CSAMPLE* pBeatInput = pIn;
    CSAMPLE* pDrumChannel = nullptr;

    if (m_channelCount == mixxx::audio::ChannelCount::stem()) {
        // We have an 8 channel soundsource. The only implemented soundsource with
        // 8ch is the NI STEM file format.
        // TODO: If we add other soundsources with 8ch, we need to rework this condition.
        //
        // For NI STEM we mix all the stems together except the first one,
        // which contains drums or beats by convention.
        count = numFrames * mixxx::audio::ChannelCount::stereo();
        pDrumChannel = SampleUtil::alloc(count);

        VERIFY_OR_DEBUG_ASSERT(pDrumChannel) {
            return false;
        }

        if (m_bpmSettings.getStemStrategy() == BeatDetectionSettings::StemStrategy::Enforced) {
            SampleUtil::copyOneStereoFromMulti(pDrumChannel, pIn, numFrames, m_channelCount, 0);
        } else {
            SampleUtil::mixMultichannelToStereo(pDrumChannel, pIn, numFrames, m_channelCount);
        }

        pBeatInput = pDrumChannel;
    } else if (m_channelCount > mixxx::audio::ChannelCount::stereo()) {
        DEBUG_ASSERT(!"Unsupported channel count");
        return false;
    }

    m_currentFrame += numFrames;
    if (m_currentFrame > m_maxFramesToProcess) {
        return true; // silently ignore all remaining samples
    }

    bool ret = m_pPlugin->processSamples(pBeatInput, count);
    if (pDrumChannel) {
        SampleUtil::free(pDrumChannel);
    }
    return ret;
}

void AnalyzerBeats::cleanup() {
    m_pPlugin.reset();
}

void AnalyzerBeats::storeResults(TrackPointer pTrack) {
    VERIFY_OR_DEBUG_ASSERT(m_pPlugin) {
        return;
    }

    if (!m_pPlugin->finalize()) {
        qWarning() << "Beat/BPM analysis failed";
        return;
    }

    mixxx::BeatsPointer pBeats;
    if (m_pPlugin->supportsBeatTracking()) {
        QVector<mixxx::audio::FramePos> beats = m_pPlugin->getBeats();

        // A beat grid needs at least two beats to have a meaningful tempo.
        // A single beat (common on very short samples) yields Bpm(Invalid)
        // and corrupts the track's beat grid.
        if (beats.size() < 2) {
            qWarning() << "AnalyzerBeats: too few beats (" << beats.size()
                       << ") - skipping beat grid storage";
            return;
        }

        // Export beats to CSV
        // uncomment to get the file created
        // exportBeatsToCsv(pTrack, beats, m_sampleRate);
        QHash<QString, QString> extraVersionInfo = getExtraVersionInfo(
                m_pluginId, m_bPreferencesFastAnalysis);
        pBeats = BeatFactory::makePreferredBeats(
                beats,
                extraVersionInfo,
                m_bPreferencesFixedTempo,
                m_sampleRate);
        qDebug() << "AnalyzerBeats plugin detected" << beats.size()
                 << "beats. Predominant BPM:"
                 << (pBeats ? pBeats->getBpmInRange(
                                      mixxx::audio::kStartFramePos,
                                      mixxx::audio::FramePos{
                                              pTrack->getDuration() *
                                              pBeats->getSampleRate()})
                            : mixxx::Bpm());
    } else {
        mixxx::Bpm bpm = m_pPlugin->getBpm();
        qDebug() << "AnalyzerBeats plugin detected constant BPM: " << bpm;
        pBeats = mixxx::Beats::fromConstTempo(m_sampleRate, mixxx::audio::kStartFramePos, bpm);
    }

    pTrack->trySetBeats(pBeats);
    // BPM SEGMENTS -> JSON & DB
    auto* pExtendedPlugin = dynamic_cast<mixxx::AnalyzerQueenMaryBeatsExtended*>(m_pPlugin.get());
    if (pExtendedPlugin) {
        if (showDebugWAnalyzerBeats) {
            qDebug() << "[AnalyzerBeats] ========================================";
            qDebug() << "[AnalyzerBeats] Track:" << pTrack->getArtist() << "-"
                     << pTrack->getTitle();
            qDebug() << "[AnalyzerBeats] Duration:" << pTrack->getDuration() << "seconds";
            qDebug() << "[AnalyzerBeats] ========================================";
        }
        QJsonArray segmentsArray = pExtendedPlugin->getBpmSegmentsJson();

        // -> DB
        if (!segmentsArray.isEmpty()) {
            QList<BpmSegmentsPointer> dbSegments;

            for (const QJsonValue& val : std::as_const(segmentsArray)) {
                if (!val.isObject()) {
                    continue;
                }

                QJsonObject obj = val.toObject();

                const double bpmStart = obj["bpm_start"].toDouble();
                const double bpmEnd = obj["bpm_end"].toDouble();
                const double duration = obj["duration"].toDouble();
                if (bpmStart <= 0.0 || bpmEnd <= 0.0 || duration <= 0.0) {
                    qWarning() << "[AnalyzerBeats] Skipping invalid BPM segment:"
                               << "bpm_start" << bpmStart
                               << "bpm_end" << bpmEnd
                               << "duration" << duration;
                    continue;
                }

                BpmSegmentsPointer pSegment(new BpmSegments(
                        obj["position"].toDouble(),
                        duration,
                        bpmStart,
                        bpmEnd,
                        obj["range_start"].toDouble(),
                        obj["range_end"].toDouble(),
                        obj["type"].toString()));
                dbSegments.append(pSegment);
            }

            // mark segments dirty and save new segments
            pTrack->deleteBpmSegments();
            if (pTrack->setBpmSegments(dbSegments)) {
                if (showDebugWAnalyzerBeats) {
                    qDebug() << "[AnalyzerBeats] Saved" << dbSegments.size()
                             << "BPM segments to track" << pTrack->getId();
                }
            }
        }
        if (showDebugWAnalyzerBeats) {
            qDebug() << "[AnalyzerBeats] Total segments:" << segmentsArray.size();
        }

        // -> JSON
        if (!segmentsArray.isEmpty()) {
            saveBpmSegmentsJson(pTrack, segmentsArray);
        }
        if (showDebugWAnalyzerBeats) {
            qDebug() << "[AnalyzerBeats] ========================================";
            qDebug() << "[AnalyzerBeats] Total segments:" << segmentsArray.size();
            qDebug() << "[AnalyzerBeats] ========================================";
        }
    }
}

bool AnalyzerBeats::exportBeatsToCsv(TrackPointer pTrack,
        const QVector<mixxx::audio::FramePos>& beats,
        double sampleRate) {
    if (!pTrack) {
        if (showDebugWAnalyzerBeats) {
            qDebug() << "[AnalyzerBeats] No track for CSV export";
        }
        return false;
    }

    if (beats.isEmpty()) {
        if (showDebugWAnalyzerBeats) {
            qDebug() << "[AnalyzerBeats] No beats to export";
        }
        return false;
    }

    QString trackIdStr = pTrack->getId().toString();
    if (trackIdStr.isEmpty()) {
        if (showDebugWAnalyzerBeats) {
            qDebug() << "[AnalyzerBeats] No track ID for CSV export";
        }
        return false;
    }

    QString bpmCurvePath = m_pConfig->getSettingsPath() + "/bpmcurve/";
    QDir saveLocation;

    if (!saveLocation.mkpath(bpmCurvePath)) {
        if (showDebugWAnalyzerBeats) {
            qDebug() << "[AnalyzerBeats] Failed to create directory:" << bpmCurvePath;
        }
    }

    QString csvFileLocation = bpmCurvePath + trackIdStr + "_beats.csv";

    QFile file(csvFileLocation);
    if (!file.open(QIODevice::WriteOnly)) {
        if (showDebugWAnalyzerBeats) {
            qDebug() << "[AnalyzerBeats] Cannot open CSV file:" << csvFileLocation;
        }
        return false;
    }

    QTextStream stream(&file);
    stream << "beat_index,time_seconds\n";

    for (int i = 0; i < beats.size(); ++i) {
        double timeSeconds = beats[i].value() / sampleRate;
        stream << i + 1 << "," << QString::number(timeSeconds, 'f', 6) << "\n";
    }

    file.close();
    if (showDebugWAnalyzerBeats) {
        qDebug() << "[AnalyzerBeats] Exported" << beats.size() << "beats to:" << csvFileLocation;
    }
    return true;
}

// Save the segments to JSON
bool AnalyzerBeats::saveBpmSegmentsJson(TrackPointer pTrack, const QJsonArray& segmentsArray) {
    QString trackIdStr = pTrack->getId().toString();
    if (trackIdStr.isEmpty()) {
        if (showDebugWAnalyzerBeats) {
            qDebug() << "[AnalyzerBeats] No valid track ID, skipping JSON save";
        }
        return false;
    }

    QJsonObject root;
    root["bpm_curve"] = segmentsArray;

    QJsonObject trackInfo;
    trackInfo["title"] = pTrack->getTitle();
    trackInfo["artist"] = pTrack->getArtist();
    trackInfo["album"] = pTrack->getAlbum();
    trackInfo["duration_seconds"] = pTrack->getDuration();
    trackInfo["offset_seconds"] = 0.0;
    trackInfo["track_id"] = trackIdStr;

    root["track"] = trackInfo;

    QString bpmCurvePath = m_pConfig->getSettingsPath() + "/bpmcurve/";
    QDir saveLocation;

    if (!saveLocation.mkpath(bpmCurvePath)) {
        if (showDebugWAnalyzerBeats) {
            qDebug() << "[AnalyzerBeats] Failed to create directory:" << bpmCurvePath;
        }
    }

    QString bpmCurveFileLocation = bpmCurvePath + pTrack->getId().toString() + ".json";

    QFile file(bpmCurveFileLocation);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
        file.close();
        if (showDebugWAnalyzerBeats) {
            qDebug() << "[AnalyzerBeats] Saved BPM curve JSON to:" << bpmCurveFileLocation;
            qDebug() << "[AnalyzerBeats] Segments:" << segmentsArray.size();
        }
        return true;
    } else {
        if (showDebugWAnalyzerBeats) {
            qDebug() << "[AnalyzerBeats] Failed to save BPM curve JSON to:" << bpmCurveFileLocation;
        }
        return false;
    }
}

// static
QHash<QString, QString> AnalyzerBeats::getExtraVersionInfo(
        const QString& pluginId, bool bPreferencesFastAnalysis) {
    QHash<QString, QString> extraVersionInfo;
    extraVersionInfo["vamp_plugin_id"] = pluginId;
    if (bPreferencesFastAnalysis) {
        extraVersionInfo["fast_analysis"] = "1";
    }
    return extraVersionInfo;
}
