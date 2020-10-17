#include "analyzer/analyzerbeats.h"

#include <QHash>
#include <QString>
#include <QVector>

#include "analyzer/constants.h"
#include "analyzer/plugins/analyzerqueenmarybeats.h"
#include "analyzer/plugins/analyzersoundtouchbeats.h"
#include "library/rekordbox/rekordboxconstants.h"
#include "track/beatfactory.h"
#include "track/beats.h"
#include "track/beatutils.h"
#include "track/track.h"

// static
QList<mixxx::AnalyzerPluginInfo> AnalyzerBeats::availablePlugins() {
    QList<mixxx::AnalyzerPluginInfo> plugins;
    // First one below is the default
    plugins.append(mixxx::AnalyzerQueenMaryBeats::pluginInfo());
    plugins.append(mixxx::AnalyzerSoundTouchBeats::pluginInfo());
    return plugins;
}

// static
mixxx::AnalyzerPluginInfo AnalyzerBeats::defaultPlugin() {
    DEBUG_ASSERT(availablePlugins().size() > 0);
    return availablePlugins().at(0);
}

AnalyzerBeats::AnalyzerBeats(UserSettingsPointer pConfig, bool enforceBpmDetection)
        : m_bpmSettings(pConfig),
          m_enforceBpmDetection(enforceBpmDetection),
          m_bPreferencesReanalyzeOldBpm(false),
          m_bPreferencesReanalyzeImported(false),
          m_bPreferencesFixedTempo(true),
          m_bPreferencesOffsetCorrection(false),
          m_bPreferencesFastAnalysis(false),
          m_iSampleRate(0),
          m_iTotalSamples(0),
          m_iMaxSamplesToProcess(0),
          m_iCurrentSample(0),
          m_iMinBpm(0),
          m_iMaxBpm(9999) {
}

bool AnalyzerBeats::initialize(TrackPointer pTrack, int sampleRate, int totalSamples) {
    if (totalSamples == 0) {
        return false;
    }

    bool bPreferencesBeatDetectionEnabled =
            m_enforceBpmDetection || m_bpmSettings.getBpmDetectionEnabled();
    if (!bPreferencesBeatDetectionEnabled) {
        qDebug() << "Beat calculation is deactivated";
        return false;
    }

    bool bpmLock = pTrack->isBpmLocked();
    if (bpmLock) {
        qDebug() << "Track is BpmLocked: Beat calculation will not start";
        return false;
    }

    m_iMinBpm = m_bpmSettings.getBpmRangeStart();
    m_iMaxBpm = m_bpmSettings.getBpmRangeEnd();

    m_bPreferencesFixedTempo = m_bpmSettings.getFixedTempoAssumption();
    m_bPreferencesOffsetCorrection = m_bpmSettings.getFixedTempoOffsetCorrection();
    m_bPreferencesReanalyzeOldBpm = m_bpmSettings.getReanalyzeWhenSettingsChange();
    m_bPreferencesReanalyzeImported = m_bpmSettings.getReanalyzeImported();
    m_bPreferencesFastAnalysis = m_bpmSettings.getFastAnalysis();

    if (availablePlugins().size() > 0) {
        m_pluginId = defaultPlugin().id;
        QString pluginId = m_bpmSettings.getBeatPluginId();
        for (const auto& info : availablePlugins()) {
            if (info.id == pluginId) {
                m_pluginId = pluginId; // configured Plug-In available
                break;
            }
        }
    }

    qDebug() << "AnalyzerBeats preference settings:"
             << "\nPlugin:" << m_pluginId
             << "\nMin/Max BPM:" << m_iMinBpm << m_iMaxBpm
             << "\nFixed tempo assumption:" << m_bPreferencesFixedTempo
             << "\nOffset correction:" << m_bPreferencesOffsetCorrection
             << "\nRe-analyze when settings change:" << m_bPreferencesReanalyzeOldBpm
             << "\nFast analysis:" << m_bPreferencesFastAnalysis;

    m_iSampleRate = sampleRate;
    m_iTotalSamples = totalSamples;
    // In fast analysis mode, skip processing after
    // kFastAnalysisSecondsToAnalyze seconds are analyzed.
    if (m_bPreferencesFastAnalysis) {
        m_iMaxSamplesToProcess =
                mixxx::kFastAnalysisSecondsToAnalyze * m_iSampleRate * mixxx::kAnalysisChannels;
    } else {
        m_iMaxSamplesToProcess = m_iTotalSamples;
    }
    m_iCurrentSample = 0;

    // if we can load a stored track don't reanalyze it
    bool bShouldAnalyze = shouldAnalyze(pTrack);

    DEBUG_ASSERT(!m_pPlugin);
    if (bShouldAnalyze) {
        if (m_pluginId == mixxx::AnalyzerQueenMaryBeats::pluginInfo().id) {
            m_pPlugin = std::make_unique<mixxx::AnalyzerQueenMaryBeats>();
        } else if (m_pluginId == mixxx::AnalyzerSoundTouchBeats::pluginInfo().id) {
            m_pPlugin = std::make_unique<mixxx::AnalyzerSoundTouchBeats>();
        } else {
            // This must not happen, because we have already verified above
            // that the PlugInId is valid
            DEBUG_ASSERT(false);
        }

        if (m_pPlugin) {
            if (m_pPlugin->initialize(sampleRate)) {
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
    int iMinBpm = m_bpmSettings.getBpmRangeStart();
    int iMaxBpm = m_bpmSettings.getBpmRangeEnd();

    bool bpmLock = pTrack->isBpmLocked();
    if (bpmLock) {
        qDebug() << "Track is BpmLocked: Beat calculation will not start";
        return false;
    }

    QString pluginID = m_bpmSettings.getBeatPluginId();
    if (pluginID.isEmpty()) {
        pluginID = defaultPlugin().id;
    }

    // If the track already has a Beats object then we need to decide whether to
    // analyze this track or not.
    mixxx::BeatsPointer pBeats = pTrack->getBeats();
    if (!pBeats) {
        return true;
    }
    if (!mixxx::Bpm::isValidValue(pBeats->getGlobalBpm().getValue())) {
        // Tracks with an invalid bpm <= 0 should be re-analyzed,
        // independent of the preference settings. We expect that
        // all tracks have a bpm > 0 when analyzed. Users that want
        // to keep their zero bpm tracks could lock them to prevent
        // this re-analysis (see the check above).
        qDebug() << "Re-analyzing track with invalid BPM despite preference settings.";
        return true;
    }
    if (pBeats->findNextBeat(mixxx::kStartFramePos)->framePosition() <= mixxx::kStartFramePos) {
        qDebug() << "First beat is 0 for grid so analyzing track to find first beat.";
        return true;
    }

    // Version check
    QString version = pBeats->getVersion();
    QString subVersion = pBeats->getSubVersion();
    QHash<QString, QString> extraVersionInfo = getExtraVersionInfo(
            pluginID,
            m_bPreferencesFastAnalysis);
    QString newVersion = BeatFactory::getPreferredVersion(
            m_bPreferencesOffsetCorrection);
    QString newSubVersion = BeatFactory::getPreferredSubVersion(
            m_bPreferencesFixedTempo,
            m_bPreferencesOffsetCorrection,
            iMinBpm,
            iMaxBpm,
            extraVersionInfo);
    if (subVersion == mixxx::rekordboxconstants::beatsSubversion) {
        return m_bPreferencesReanalyzeImported;
    }
    if (version == newVersion && subVersion == newSubVersion) {
        // If the version and settings have not changed then if the world is
        // sane, re-analyzing will do nothing.
        return false;
    }
    // Beat grid exists but version and settings differ
    if (!m_bPreferencesReanalyzeOldBpm) {
        qDebug() << "Beat calculation skips analyzing because the track has"
                 << "a BPM computed by a previous Mixxx version and user"
                 << "preferences indicate we should not change it.";
        return false;
    }

    return true;
}

bool AnalyzerBeats::processSamples(const CSAMPLE *pIn, const int iLen) {
    VERIFY_OR_DEBUG_ASSERT(m_pPlugin) {
        return false;
    }

    m_iCurrentSample += iLen;
    if (m_iCurrentSample > m_iMaxSamplesToProcess) {
        return true; // silently ignore all remaining samples
    }

    return m_pPlugin->processSamples(pIn, iLen);
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

    mixxx::BeatsInternal newBeats;
    if (m_pPlugin->supportsBeatTracking()) {
        QVector<double> beats = m_pPlugin->getBeats();
        QHash<QString, QString> extraVersionInfo = getExtraVersionInfo(
                m_pluginId, m_bPreferencesFastAnalysis);
        newBeats = BeatFactory::makePreferredBeats(
                pTrack,
                beats,
                extraVersionInfo,
                m_bPreferencesFixedTempo,
                m_bPreferencesOffsetCorrection,
                m_iTotalSamples,
                m_iMinBpm,
                m_iMaxBpm);
        qDebug() << "AnalyzerBeats plugin detected" << beats.size()
                 << "beats. Average BPM:" << newBeats.getGlobalBpm();
    } else {
        auto bpm = mixxx::Bpm(m_pPlugin->getBpm());
        qDebug() << "AnalyzerBeats plugin detected constant BPM: " << bpm;
        pTrack->setBpm(bpm.getValue());
    }

    mixxx::BeatsPointer pCurrentBeats = pTrack->getBeats();

    // If the track has no beats object then set our newly generated one
    // regardless of beat lock.
    if (!pCurrentBeats) {
        pTrack->setBeats(newBeats);
        return;
    }

    // If the track received the beat lock while we were analyzing it then we
    // abort setting it.
    if (pTrack->isBpmLocked()) {
        qDebug() << "Track was BPM-locked as we were analyzing it. Aborting analysis.";
        return;
    }

    // If the user prefers to replace old beatgrids with newly generated ones or
    // the old beatgrid has 0-bpm then we replace it.
    bool zeroCurrentBpm = pCurrentBeats->getGlobalBpm().getValue() == 0.0;
    if (m_bPreferencesReanalyzeOldBpm || zeroCurrentBpm) {
        if (zeroCurrentBpm) {
            qDebug() << "Replacing 0-BPM beatgrid with a" << newBeats.getGlobalBpm()
                     << "beatgrid.";
        }
        pTrack->setBeats(newBeats);
        return;
    }

    // If we got here then the user doesn't want to replace the beatgrid but
    // since the first beat is zero we'll apply the offset we just detected.
    mixxx::FramePos currentFirstBeat =
            pCurrentBeats->getBeatAtIndex(mixxx::kFirstBeatIndex)->framePosition();
    mixxx::FramePos newFirstBeat = newBeats.getBeatAtIndex(mixxx::kFirstBeatIndex)->framePosition();
    if (currentFirstBeat == mixxx::kStartFramePos && newFirstBeat > mixxx::kStartFramePos) {
        const double translateDuration =
                (newFirstBeat - currentFirstBeat) / pTrack->getSampleRate();
        pCurrentBeats->translateBySeconds(translateDuration);
    }
}

// static
QHash<QString, QString> AnalyzerBeats::getExtraVersionInfo(
        QString pluginId, bool bPreferencesFastAnalysis) {
    QHash<QString, QString> extraVersionInfo;
    extraVersionInfo["vamp_plugin_id"] = pluginId;
    if (bPreferencesFastAnalysis) {
        extraVersionInfo["fast_analysis"] = "1";
    }
    return extraVersionInfo;
}
