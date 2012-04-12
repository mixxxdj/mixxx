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

static bool sDebug = false;

AnalyserBeats::AnalyserBeats(ConfigObject<ConfigValue> *_config)
        : m_pConfig(_config),
          m_pVamp(NULL),
          m_bShouldAnalyze(false),
          m_bPreferencesBeatDetectionEnabled(true),
          m_bPreferencesReanalyzeOldBpm(false),
          m_bPreferencesFixedTempo(true),
          m_bPreferencesOffsetCorrection(false),
          m_iSampleRate(0),
          m_iTotalSamples(0),
          m_iMinBpm(0),
          m_iMaxBpm(9999) {
}

AnalyserBeats::~AnalyserBeats(){
}

void AnalyserBeats::initialise(TrackPointer tio, int sampleRate, int totalSamples) {
    m_bShouldAnalyze = false;
    if (totalSamples == 0)
        return;

    QString library = m_pConfig->getValueString(
        ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYSER_BEAT_LIBRARY));
    QString pluginID = m_pConfig->getValueString(
        ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYSER_BEAT_PLUGIN_ID));

    m_iMinBpm = m_pConfig->getValueString(ConfigKey(BPM_CONFIG_KEY, BPM_RANGE_START)).toInt();
    m_iMaxBpm = m_pConfig->getValueString(ConfigKey(BPM_CONFIG_KEY, BPM_RANGE_END)).toInt();
    bool allow_above = static_cast<bool>(m_pConfig->getValueString(
        ConfigKey(BPM_CONFIG_KEY, BPM_ABOVE_RANGE_ENABLED)).toInt());
    if (allow_above) {
        m_iMinBpm = 0;
        m_iMaxBpm = 9999;
    }

    m_bPreferencesBeatDetectionEnabled = static_cast<bool>(
        m_pConfig->getValueString(
            ConfigKey(BPM_CONFIG_KEY, BPM_DETECTION_ENABLED)).toInt());
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

    if (!m_bPreferencesBeatDetectionEnabled) {
        qDebug() << "Beat calculation is deactivated";
        m_bShouldAnalyze = false;
        return;
    }

    bool bpmLock = tio->hasBpmLock();
    if (bpmLock) {
        qDebug() << "Track is BpmLocked: Beat calculation will not start";
        m_bShouldAnalyze = false;
        return;
    }

    // At first start config for QM and Vamp does not exist --> set default
    if (library.isEmpty() || library.isNull())
        library = "libmixxxminimal";
    if (pluginID.isEmpty() || pluginID.isNull())
        pluginID="qm-tempotracker:0";
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

        QString newVersion = BeatFactory::getPreferredVersion(
            m_bPreferencesFixedTempo, m_bPreferencesOffsetCorrection,
            m_iMinBpm, m_iMaxBpm, extraVersionInfo);
        QString newSubVersion = BeatFactory::getPreferredSubVersion(
            m_bPreferencesFixedTempo, m_bPreferencesOffsetCorrection,
            m_iMinBpm, m_iMaxBpm, extraVersionInfo);

        m_bShouldAnalyze = m_bPreferencesReanalyzeOldBpm;
        if (!m_bPreferencesReanalyzeOldBpm) {
            qDebug() << "Beat calculation skips analyzing because the track has"
                     << "a BPM computed by a previous Mixxx version and user"
                     << "preferences indicate we should not change it.";
        }

        // Override preference if the BPM is 0.
        if (tio->getBeats()->getBpm() == 0.0) {
            m_bShouldAnalyze = true;
            qDebug() << "BPM is 0 for track so re-analyzing despite preference settings.";
        } else if (version == newVersion && subVersion == newSubVersion) {
            // Do not re-analyze if the settings are the same and the BPM is not
            // 0.
            qDebug() << "Beat sub-version has not changed since previous analysis so not analyzing.";
            m_bShouldAnalyze = false;
        }
    }

    if (!m_bShouldAnalyze) {
        qDebug() << "Beat calculation will not start";
        return;
    }
    qDebug() << "Beat calculation started with plugin" << pluginID;

    m_pVamp = new VampAnalyser(m_pConfig);
    m_bShouldAnalyze = m_pVamp->Init(library, pluginID, m_iSampleRate, totalSamples);
    if (!m_bShouldAnalyze) {
        delete m_pVamp;
        m_pVamp = NULL;
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

void AnalyserBeats::finalise(TrackPointer tio) {
    if (!m_bShouldAnalyze || m_pVamp == NULL) {
        return;
    }

    // Call End() here, because the number of total samples may have been
    // estimated incorrectly.
    bool success = m_pVamp->End();
    qDebug() << "Beat Calculation" << (success ? "complete" : "failed");

    QVector<double> beats = m_pVamp->GetInitFramesVector();
    if (!beats.isEmpty()) {
        QHash<QString, QString> extraVersionInfo;
        extraVersionInfo["vamp_plugin_id"] = m_pluginId;

        BeatsPointer pBeats = BeatFactory::makePreferredBeats(
            tio, beats, extraVersionInfo,
            m_bPreferencesFixedTempo, m_bPreferencesOffsetCorrection,
            m_iSampleRate, m_iTotalSamples,
            m_iMinBpm, m_iMaxBpm);
        tio->setBeats(pBeats);
        tio->setBpm(pBeats->getBpm());
    } else {
        qDebug() << "Could not detect beat positions from Vamp.";
    }

    delete m_pVamp;
    m_pVamp = NULL;
}
