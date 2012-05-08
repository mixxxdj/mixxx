/*
 * analyserrgain.cpp
 *
 *  Created on: 19/apr/2011
 *      Author: vittorio
 */

#include <QtDebug>
#include <QList>
#include <QVector>
#include <QString>
#include <QStringList>
#include <math.h>
#include "analysergainvamp.h"

AnalyserGainVamp::AnalyserGainVamp(ConfigObject<ConfigValue>* _config) {
    m_pConfig = _config;
}

AnalyserGainVamp::~AnalyserGainVamp() {
}

bool AnalyserGainVamp::initialise(TrackPointer tio, int sampleRate, int totalSamples) {
    m_pVamp = new VampAnalyser(m_pConfig);
    m_bShouldAnalyze = false;
    bool bAnalyserEnabled = static_cast<bool>(
        m_pConfig->getValueString(ConfigKey("[ReplayGain]","ReplayGainAnalyserEnabled")).toInt());

    float fReplayGain = tio->getReplayGain();
    if (totalSamples == 0 || fReplayGain != 0 || !bAnalyserEnabled) {
        //qDebug() << "Replaygain Analyser will not start.";
        //if (fReplayGain != 0 ) qDebug() << "Found a ReplayGain value of " << 20*log10(fReplayGain) << "dB for track :" <<(tio->getFilename());
        return false;
    }

    m_bShouldAnalyze = m_pVamp->Init("libmixxxminimal", "replaygain:0",
                                     sampleRate, totalSamples, false);
    if (!m_bShouldAnalyze) {
        qDebug() << "Failed to init Vamp Replay Gain Analyser";
        delete m_pVamp;
        m_pVamp = NULL;
    }
    return m_bShouldAnalyze;
}

void AnalyserGainVamp::process(const CSAMPLE *pIn, const int iLen) {
    if (!m_bShouldAnalyze) {
        return;
    }
    m_bShouldAnalyze = m_pVamp->Process(pIn, iLen);
    if (!m_bShouldAnalyze) {
        delete m_pVamp;
        m_pVamp = NULL;
    }
}

void AnalyserGainVamp::finalise(TrackPointer tio) {
    if (!m_bShouldAnalyze) {
        return;
    }

    QVector<double> values = m_pVamp->GetFirstValuesVector();
    if (!values.isEmpty()) {
        //qDebug() << "Found a ReplayGain value of" << pow(10,values[0]/20);
        float fReplayGain_Result = pow(10, values[0]/20);
        tio->setReplayGain(fReplayGain_Result);
    }

    // Should this come before the GetFirstValuesVector call?
    m_bShouldAnalyze = m_pVamp->End();

    if (m_bShouldAnalyze) {
        qDebug() << "ReplayGain detection complete";
    } else {
        qDebug() << "Error in ReplayGain detection";
    }

    delete m_pVamp;
    m_pVamp = NULL;
}
