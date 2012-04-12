/*
 * analyserbeats.cpp
 *
 *  Created on: 16/mar/2011
 *      Author: vittorio
 */

#include <QtDebug>
#include <QVector>
#include <QString>

#include "trackinfoobject.h"
#include "track/beatmap.h"
#include "track/beatfactory.h"
#include "analyserbeats.h"
#include "track/beatutils.h"

static bool sDebug = false;

AnalyserBeats::AnalyserBeats(ConfigObject<ConfigValue> *_config) {
    m_pConfigAVT = _config;
    m_bShouldAnalyze = false;
    m_iSampleRate = 0;
    m_iTotalSamples = 0;
}

AnalyserBeats::~AnalyserBeats(){
}

void AnalyserBeats::initialise(TrackPointer tio, int sampleRate, int totalSamples) {
    m_bShouldAnalyze = false;
    if (totalSamples == 0)
        return;
    m_iMinBpm = m_pConfigAVT->getValueString(ConfigKey("[BPM]","BPMRangeStart")).toInt();
    m_iMaxBpm = m_pConfigAVT->getValueString(ConfigKey("[BPM]","BPMRangeEnd")).toInt();
    int allow_above = m_pConfigAVT->getValueString(ConfigKey("[BPM]","BPMAboveRangeEnabled")).toInt();
    if (allow_above) {
        m_iMinBpm = 0;
        m_iMaxBpm = 9999;
    }
    QString library = m_pConfigAVT->getValueString(ConfigKey("[Vamp]","AnalyserBeatLibrary"));
    QString pluginID = m_pConfigAVT->getValueString(ConfigKey("[Vamp]","AnalyserBeatPluginID"));

    m_bPreferencesBeatDetectionEnabled = static_cast<bool>(
        m_pConfigAVT->getValueString(ConfigKey("[Vamp]","AnalyserBeatEnabled")).toInt());
    m_bPreferencesReanalyzeOldBpm = static_cast<bool>(
        m_pConfigAVT->getValueString(ConfigKey("[Vamp]","ReanalyzeOldBPM")).toInt());
    m_bPreferencesFixedTempo = static_cast<bool>(
        m_pConfigAVT->getValueString(ConfigKey("[Vamp]","AnalyserBeatFixedTempo")).toInt());
    m_bPreferencesOffsetCorrection = static_cast<bool>(
        m_pConfigAVT->getValueString(ConfigKey("[Vamp]","AnalyserBeatOffset")).toInt());

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

    m_iSampleRate = sampleRate;
    m_iTotalSamples = totalSamples;

    QString correction;
    if (m_bPreferencesFixedTempo) {
        if (m_bPreferencesOffsetCorrection) {
            correction = "offset";
        } else {
            correction = "const";
        }
    } else {
        correction = "none";
    }
    // Check if BPM algorithm has changed
    QString pluginname = pluginID;
    pluginname.replace(QString(":"),QString("_output="));
    m_sSubver = QString("plugin=%1_beats_correction=%2").arg(pluginname,correction);

    // If the track already has a Beats object then we need to decide whether to
    // analyze this track or not.
    BeatsPointer pBeats = tio->getBeats();
    if (pBeats) {
        QString version = pBeats->getVersion();
        QString subVersion = pBeats->getSubVersion();

        m_bShouldAnalyze = m_bPreferencesReanalyzeOldBpm;
        if (!m_bPreferencesReanalyzeOldBpm) {
            qDebug() << "Beat calculation skips analyzing because the track has a BPM computed by a previous Mixxx version.";
        }

        // Override preference if the BPM is 0.
        if (tio->getBeats()->getBpm() == 0.0) {
            m_bShouldAnalyze = true;
            qDebug() << "BPM is 0 for track so re-analyzing despite preference settings.";
        } else if (subVersion == m_sSubver) {
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

    m_pVamp = new VampAnalyser(m_pConfigAVT);
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
        BeatsPointer pBeats = BeatFactory::makeBeatMap(
            tio, beats, m_sSubver,
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
