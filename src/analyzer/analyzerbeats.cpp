#include "analyzer/analyzerbeats.h"

#include <QtDebug>
#include <QVector>
#include <QHash>
#include <QString>

#include "analyzer/constants.h"
#include "analyzer/plugins/analyzersoundtouchbeats.h"
#include "analyzer/plugins/analyzerqueenmarybeats.h"
#include "track/beatfactory.h"
#include "track/beatmap.h"
#include "track/beatutils.h"
#include "track/track.h"

// static
QList<mixxx::AnalyzerPluginInfo> AnalyzerBeats::availablePlugins() {
    QList<mixxx::AnalyzerPluginInfo> plugins;
    plugins.append(mixxx::AnalyzerSoundTouchBeats::pluginInfo());
    plugins.append(mixxx::AnalyzerQueenMaryBeats::pluginInfo());
    return plugins;
}

AnalyzerBeats::AnalyzerBeats(UserSettingsPointer pConfig, bool enforceBpmDetection)
        : m_bpmSettings(pConfig),
          m_enforceBpmDetection(enforceBpmDetection),
          m_bPreferencesReanalyzeOldBpm(false),
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

bool AnalyzerBeats::initialize(TrackPointer tio, int sampleRate, int totalSamples) {
    if (totalSamples == 0) {
        return false;
    }

    bool bPreferencesBeatDetectionEnabled =
            m_enforceBpmDetection || m_bpmSettings.getBpmDetectionEnabled();
    if (!bPreferencesBeatDetectionEnabled) {
        qDebug() << "Beat calculation is deactivated";
        return false;
    }

    bool bpmLock = tio->isBpmLocked();
    if (bpmLock) {
        qDebug() << "Track is BpmLocked: Beat calculation will not start";
        return false;
    }

    if (m_bpmSettings.getAllowBpmAboveRange()) {
        m_iMinBpm = 0;
        m_iMaxBpm = 9999;
    } else {
        m_iMinBpm = m_bpmSettings.getBpmRangeStart();
        m_iMaxBpm = m_bpmSettings.getBpmRangeEnd();
    }

    m_bPreferencesFixedTempo = m_bpmSettings.getFixedTempoAssumption();
    m_bPreferencesOffsetCorrection = m_bpmSettings.getFixedTempoOffsetCorrection();
    m_bPreferencesReanalyzeOldBpm = m_bpmSettings.getReanalyzeWhenSettingsChange();
    m_bPreferencesFastAnalysis = m_bpmSettings.getFastAnalysis();
    m_pluginId = m_bpmSettings.getBeatPluginId();

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
        m_iMaxSamplesToProcess = mixxx::kFastAnalysisSecondsToAnalyze * m_iSampleRate * mixxx::kAnalysisChannels;
    } else {
        m_iMaxSamplesToProcess = m_iTotalSamples;
    }
    m_iCurrentSample = 0;

    // if we can load a stored track don't reanalyze it
    bool bShouldAnalyze = !isDisabledOrLoadStoredSuccess(tio);

    if (bShouldAnalyze) {
        if (m_pluginId == mixxx::AnalyzerSoundTouchBeats::pluginInfo().id) {
            m_pPlugin = std::make_unique<mixxx::AnalyzerSoundTouchBeats>();
        } else if (m_pluginId == mixxx::AnalyzerQueenMaryBeats::pluginInfo().id) {
            m_pPlugin = std::make_unique<mixxx::AnalyzerQueenMaryBeats>();
        } else {
            m_pPlugin = std::make_unique<mixxx::AnalyzerQueenMaryBeats>();
        }
        bShouldAnalyze = m_pPlugin->initialize(m_iSampleRate);
    }

    if (bShouldAnalyze) {
        qDebug() << "Beat calculation started with plugin" << m_pluginId;
    } else {
        qDebug() << "Beat calculation will not start";
        m_pPlugin.reset();
    }

    return bShouldAnalyze;
}

bool AnalyzerBeats::isDisabledOrLoadStoredSuccess(TrackPointer tio) const {
    int iMinBpm;
    int iMaxBpm;
    if (m_bpmSettings.getAllowBpmAboveRange()) {
        iMinBpm = 0;
        iMaxBpm = 9999;
    } else {
        iMinBpm = m_bpmSettings.getBpmRangeStart();
        iMaxBpm = m_bpmSettings.getBpmRangeEnd();
    }

    bool bpmLock = tio->isBpmLocked();
    if (bpmLock) {
        qDebug() << "Track is BpmLocked: Beat calculation will not start";
        return true;
    }

    QString pluginID = m_bpmSettings.getBeatPluginId();

    // If the track already has a Beats object then we need to decide whether to
    // analyze this track or not.
    BeatsPointer pBeats = tio->getBeats();
    if (pBeats) {
        QString version = pBeats->getVersion();
        QString subVersion = pBeats->getSubVersion();

        QHash<QString, QString> extraVersionInfo = getExtraVersionInfo(
            pluginID, m_bPreferencesFastAnalysis);
        QString newVersion = BeatFactory::getPreferredVersion(
            m_bPreferencesOffsetCorrection);
        QString newSubVersion = BeatFactory::getPreferredSubVersion(
            m_bPreferencesFixedTempo, m_bPreferencesOffsetCorrection,
            iMinBpm, iMaxBpm, extraVersionInfo);

        if (version == newVersion && subVersion == newSubVersion) {
            // If the version and settings have not changed then if the world is
            // sane, re-analyzing will do nothing.
            return true;
        } else if (m_bPreferencesReanalyzeOldBpm) {
            return false;
        } else if (pBeats->getBpm() == 0.0) {
            qDebug() << "BPM is 0 for track so re-analyzing despite preference settings.";
            return false;
        } else if (pBeats->findNextBeat(0) <= 0.0) {
            qDebug() << "First beat is 0 for grid so analyzing track to find first beat.";
            return false;
        } else {
            qDebug() << "Beat calculation skips analyzing because the track has"
                     << "a BPM computed by a previous Mixxx version and user"
                     << "preferences indicate we should not change it.";
            return true;
        }
    } else {
        // If we got here, we want to analyze this track.
        return false;
    }
}

void AnalyzerBeats::process(const CSAMPLE *pIn, const int iLen) {
    if (!m_pPlugin) {
        return;
    }

    m_iCurrentSample += iLen;
    if (m_iCurrentSample > m_iMaxSamplesToProcess) {
        return;
    }

    bool success = m_pPlugin->process(pIn, iLen);
    if (!success) {
        m_pPlugin.reset();
    }
}

void AnalyzerBeats::cleanup(TrackPointer tio) {
    Q_UNUSED(tio);
    m_pPlugin.reset();
}

void AnalyzerBeats::finalize(TrackPointer tio) {
    if (!m_pPlugin) {
        return;
    }

    bool success = m_pPlugin->finalize();
    qDebug() << "Beat Calculation" << (success ? "complete" : "failed");

    if (!success) {
        m_pPlugin.reset();
        return;
    }

    BeatsPointer pBeats;
    if (m_pPlugin->supportsBeatTracking()) {
        QVector<double> beats = m_pPlugin->getBeats();
        QHash<QString, QString> extraVersionInfo = getExtraVersionInfo(
            m_pluginId, m_bPreferencesFastAnalysis);
        pBeats = BeatFactory::makePreferredBeats(
            *tio, beats, extraVersionInfo,
            m_bPreferencesFixedTempo, m_bPreferencesOffsetCorrection,
            m_iSampleRate, m_iTotalSamples,
            m_iMinBpm, m_iMaxBpm);
        qDebug() << "AnalyzerBeats plugin detected" << beats.size()
                 << "beats. Average BPM:" << (pBeats ? pBeats->getBpm() : 0.0);
    } else {
        float bpm = m_pPlugin->getBpm();
        qDebug() << "AnalyzerBeats plugin detected constant BPM: " << bpm;
        pBeats = BeatFactory::makeBeatGrid(*tio, bpm, 0.0f);
    }
    m_pPlugin.reset();

    BeatsPointer pCurrentBeats = tio->getBeats();

    // If the track has no beats object then set our newly generated one
    // regardless of beat lock.
    if (!pCurrentBeats) {
        tio->setBeats(pBeats);
        return;
    }

    // If the track received the beat lock while we were analyzing it then we
    // abort setting it.
    if (tio->isBpmLocked()) {
        qDebug() << "Track was BPM-locked as we were analyzing it. Aborting analysis.";
        return;
    }

    // If the user prefers to replace old beatgrids with newly generated ones or
    // the old beatgrid has 0-bpm then we replace it.
    bool zeroCurrentBpm = pCurrentBeats->getBpm() == 0.0;
    if (m_bPreferencesReanalyzeOldBpm || zeroCurrentBpm) {
        if (zeroCurrentBpm) {
            qDebug() << "Replacing 0-BPM beatgrid with a" << pBeats->getBpm()
                     << "beatgrid.";
        }
        tio->setBeats(pBeats);
        return;
    }

    // If we got here then the user doesn't want to replace the beatgrid but
    // since the first beat is zero we'll apply the offset we just detected.
    double currentFirstBeat = pCurrentBeats->findNextBeat(0);
    double newFirstBeat = pBeats->findNextBeat(0);
    if (currentFirstBeat == 0.0 && newFirstBeat > 0) {
        pCurrentBeats->translate(newFirstBeat);
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
