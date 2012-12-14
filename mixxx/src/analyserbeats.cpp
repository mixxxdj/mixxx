/*
 * analyserbeats.cpp
 *
 *  Created on: 16/mar/2011
 *      Author: vittorio
 */

#include <QtDebug>
#include <QVector>
#include <QHash>
#include <QString>

#include "trackinfoobject.h"
#include "track/beatmap.h"
#include "track/beatfactory.h"
#include "analyserbeats.h"
#include "track/beatutils.h"
#include "track/beat_preferences.h"

AnalyserBeats::AnalyserBeats(ConfigObject<ConfigValue> *_config)
        : m_pConfig(_config),
          m_pVamp(NULL),
          m_bShouldAnalyze(false),
          m_bPreferencesReanalyzeOldBpm(false),
          m_bPreferencesFixedTempo(true),
          m_bPreferencesOffsetCorrection(false),
          m_bPreferencesFastAnalysis(false),
          m_iSampleRate(0),
          m_iTotalSamples(0),
          m_iMinBpm(0),
          m_iMaxBpm(9999) {
}

AnalyserBeats::~AnalyserBeats(){
}

bool AnalyserBeats::initialise(TrackPointer tio, int sampleRate, int totalSamples) {
    m_bShouldAnalyze = false;
    if (totalSamples == 0)
        return false;

    bool bPreferencesBeatDetectionEnabled = static_cast<bool>(
        m_pConfig->getValueString(
            ConfigKey(BPM_CONFIG_KEY, BPM_DETECTION_ENABLED)).toInt());
    if (!bPreferencesBeatDetectionEnabled) {
        qDebug() << "Beat calculation is deactivated";
        m_bShouldAnalyze = false;
        return false;
    }

    bool bpmLock = tio->hasBpmLock();
    if (bpmLock) {
        qDebug() << "Track is BpmLocked: Beat calculation will not start";
        m_bShouldAnalyze = false;
        return false;
    }

    bool allow_above = static_cast<bool>(m_pConfig->getValueString(
        ConfigKey(BPM_CONFIG_KEY, BPM_ABOVE_RANGE_ENABLED)).toInt());
    if (allow_above) {
        m_iMinBpm = 0;
        m_iMaxBpm = 9999;
    } else {
        m_iMinBpm = m_pConfig->getValueString(ConfigKey(BPM_CONFIG_KEY, BPM_RANGE_START)).toInt();
        m_iMaxBpm = m_pConfig->getValueString(ConfigKey(BPM_CONFIG_KEY, BPM_RANGE_END)).toInt();
    }

    m_bPreferencesFixedTempo = static_cast<bool>(
        m_pConfig->getValueString(
            ConfigKey(BPM_CONFIG_KEY, BPM_FIXED_TEMPO_ASSUMPTION)).toInt());
    m_bPreferencesOffsetCorrection = static_cast<bool>(
        m_pConfig->getValueString(
            ConfigKey(BPM_CONFIG_KEY, BPM_FIXED_TEMPO_OFFSET_CORRECTION)).toInt());
    m_bPreferencesReanalyzeOldBpm = static_cast<bool>(
        m_pConfig->getValueString(
            ConfigKey(BPM_CONFIG_KEY, BPM_REANALYZE_WHEN_SETTINGS_CHANGE)).toInt());
    m_bPreferencesFastAnalysis = static_cast<bool>(
        m_pConfig->getValueString(
            ConfigKey(BPM_CONFIG_KEY, BPM_FAST_ANALYSIS_ENABLED)).toInt());


    QString library = m_pConfig->getValueString(
        ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYSER_BEAT_LIBRARY));
    QString pluginID = m_pConfig->getValueString(
        ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYSER_BEAT_PLUGIN_ID));

    // At first start config for QM and Vamp does not exist --> set default
    if (library.isEmpty() || library.isNull()) {
        library = "libmixxxminimal";
    }
    if (pluginID.isEmpty() || pluginID.isNull()) {
        pluginID="qm-tempotracker:0";
    }
    m_pluginId = pluginID;

    m_iSampleRate = sampleRate;
    m_iTotalSamples = totalSamples;

    // If the track already has a Beats object then we need to decide whether to
    // analyze this track or not.
    BeatsPointer pBeats = tio->getBeats();
    if (pBeats) {
        QString version = pBeats->getVersion();
        QString subVersion = pBeats->getSubVersion();

        QHash<QString, QString> extraVersionInfo;
        extraVersionInfo["vamp_plugin_id"] = pluginID;
        if (m_bPreferencesFastAnalysis) {
            extraVersionInfo["fast_analysis"] = "1";
        }

        QString newVersion = BeatFactory::getPreferredVersion(
                m_bPreferencesFixedTempo);
        QString newSubVersion = BeatFactory::getPreferredSubVersion(
                m_bPreferencesFixedTempo, m_bPreferencesOffsetCorrection,
                m_iMinBpm, m_iMaxBpm, extraVersionInfo);

        if (version == newVersion && subVersion == newSubVersion) {
            // If the version and settings have not changed then if the world is
            // sane, re-analyzing will do nothing.
            m_bShouldAnalyze = false;
            qDebug() << "Beat sub-version has not changed since previous analysis so not analyzing.";
        } else if (m_bPreferencesReanalyzeOldBpm) {
            m_bShouldAnalyze = true;
        } else if (pBeats->getBpm() == 0.0) {
            m_bShouldAnalyze = true;
            qDebug() << "BPM is 0 for track so re-analyzing despite preference settings.";
        } else if (pBeats->findNextBeat(0) <= 0.0) {
            m_bShouldAnalyze = true;
            qDebug() << "First beat is 0 for grid so analyzing track to find first beat.";
        } else {
            m_bShouldAnalyze = false;
            qDebug() << "Beat calculation skips analyzing because the track has"
                     << "a BPM computed by a previous Mixxx version and user"
                     << "preferences indicate we should not change it.";
        }
    } else {
        // If we got here, we think we may want to analyze this track.
        m_bShouldAnalyze = true;
    }


    if (!m_bShouldAnalyze) {
        qDebug() << "Beat calculation will not start";
        return false;
    }
    qDebug() << "Beat calculation started with plugin" << pluginID;

    m_pVamp = new VampAnalyser(m_pConfig);
    m_bShouldAnalyze = m_pVamp->Init(library, pluginID, m_iSampleRate, totalSamples,
                                     m_bPreferencesFastAnalysis);
    if (!m_bShouldAnalyze) {
        delete m_pVamp;
        m_pVamp = NULL;
    }
    return m_bShouldAnalyze;
}

bool AnalyserBeats::loadStored(TrackPointer tio) const {
    int iMinBpm;
    int iMaxBpm;

    bool allow_above = static_cast<bool>(m_pConfig->getValueString(
        ConfigKey(BPM_CONFIG_KEY, BPM_ABOVE_RANGE_ENABLED)).toInt());
    if (allow_above) {
        iMinBpm = 0;
        iMaxBpm = 9999;
    } else {
        iMinBpm = m_pConfig->getValueString(ConfigKey(BPM_CONFIG_KEY, BPM_RANGE_START)).toInt();
        iMaxBpm = m_pConfig->getValueString(ConfigKey(BPM_CONFIG_KEY, BPM_RANGE_END)).toInt();
    }

    bool bPreferencesFixedTempo = static_cast<bool>(
        m_pConfig->getValueString(
            ConfigKey(BPM_CONFIG_KEY, BPM_FIXED_TEMPO_ASSUMPTION)).toInt());
    bool bPreferencesOffsetCorrection = static_cast<bool>(
        m_pConfig->getValueString(
            ConfigKey(BPM_CONFIG_KEY, BPM_FIXED_TEMPO_OFFSET_CORRECTION)).toInt());
    bool bPreferencesReanalyzeOldBpm = static_cast<bool>(
        m_pConfig->getValueString(
            ConfigKey(BPM_CONFIG_KEY, BPM_REANALYZE_WHEN_SETTINGS_CHANGE)).toInt());
    bool bPreferencesFastAnalysis = static_cast<bool>(
        m_pConfig->getValueString(
            ConfigKey(BPM_CONFIG_KEY, BPM_FAST_ANALYSIS_ENABLED)).toInt());

    bool bpmLock = tio->hasBpmLock();
    if (bpmLock) {
        qDebug() << "Track is BpmLocked: Beat calculation will not start";
        return true;
    }

    QString library = m_pConfig->getValueString(
        ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYSER_BEAT_LIBRARY));
    QString pluginID = m_pConfig->getValueString(
        ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYSER_BEAT_PLUGIN_ID));

    // At first start config for QM and Vamp does not exist --> set default
    if (library.isEmpty() || library.isNull())
        library = "libmixxxminimal";
    if (pluginID.isEmpty() || pluginID.isNull())
        pluginID="qm-tempotracker:0";

    // If the track already has a Beats object then we need to decide whether to
    // analyze this track or not.
    BeatsPointer pBeats = tio->getBeats();
    if (pBeats) {
        QString version = pBeats->getVersion();
        QString subVersion = pBeats->getSubVersion();

        QHash<QString, QString> extraVersionInfo;
        extraVersionInfo["vamp_plugin_id"] = pluginID;
        if (bPreferencesFastAnalysis) {
            extraVersionInfo["fast_analysis"] = "1";
        }

        QString newVersion = BeatFactory::getPreferredVersion(
            bPreferencesOffsetCorrection);
        QString newSubVersion = BeatFactory::getPreferredSubVersion(
            bPreferencesFixedTempo, bPreferencesOffsetCorrection,
            iMinBpm, iMaxBpm, extraVersionInfo);

        if (version == newVersion && subVersion == newSubVersion) {
            // If the version and settings have not changed then if the world is
            // sane, re-analyzing will do nothing.
            qDebug() << "Beat sub-version has not changed since previous analysis so not analyzing.";
            return true;
        } else if (bPreferencesReanalyzeOldBpm) {
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
        // If we got here, we think we may want to analyze this track.
        return false;
    }
}


void AnalyserBeats::process(const CSAMPLE *pIn, const int iLen) {
    if (!m_bShouldAnalyze || m_pVamp == NULL)
        return;
    m_bShouldAnalyze = m_pVamp->Process(pIn, iLen);
    if (!m_bShouldAnalyze) {
        delete m_pVamp;
        m_pVamp = NULL;
    }
}

void AnalyserBeats::cleanup(TrackPointer tio)
{
    Q_UNUSED(tio);
    if (!m_bShouldAnalyze) {
        return;
    }
    delete m_pVamp;
    m_pVamp = NULL;
}

void AnalyserBeats::finalise(TrackPointer tio) {
    if (!m_bShouldAnalyze || m_pVamp == NULL) {
        return;
    }

    // Call End() here, because the number of total samples may have been
    // estimated incorrectly.
    bool success = m_pVamp->End();
    qDebug() << "Beat Calculation" << (success ? "complete" : "failed");

    QVector<double> beats = m_pVamp->GetInitFramesVector();
    delete m_pVamp;
    m_pVamp = NULL;

    if (beats.isEmpty()) {
        qDebug() << "Could not detect beat positions from Vamp.";
        return;
    }

    QHash<QString, QString> extraVersionInfo;
    extraVersionInfo["vamp_plugin_id"] = m_pluginId;
    if (m_bPreferencesFastAnalysis) {
        extraVersionInfo["fast_analysis"] = "1";
    }

    BeatsPointer pBeats = BeatFactory::makePreferredBeats(
        tio, beats, extraVersionInfo,
        m_bPreferencesFixedTempo, m_bPreferencesOffsetCorrection,
        m_iSampleRate, m_iTotalSamples,
        m_iMinBpm, m_iMaxBpm);

    BeatsPointer pCurrentBeats = tio->getBeats();

    // If the track has no beats object then set our newly generated one
    // regardless of beat lock.
    if (!pCurrentBeats) {
        tio->setBeats(pBeats);
        return;
    }

    if (tio->hasBpmLock()) {
        qDebug() << "Track was BPM-locked as we were analysing it. Aborting analysis.";
        return;
    }

    // If the user prefers to replace old beatgrids with newly generated ones or
    // the old beatgrid has 0-bpm then we replace it.
    bool zeroCurrentBpm = pCurrentBeats->getBpm() == 0.0f;
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
