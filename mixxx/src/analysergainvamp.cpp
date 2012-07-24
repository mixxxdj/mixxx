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

AnalyserGainVamp::AnalyserGainVamp(ConfigObject<ConfigValue> *_config) {
    m_pConfigAVR = _config;
}

AnalyserGainVamp::~AnalyserGainVamp() {
}

bool AnalyserGainVamp::initialise(TrackPointer tio, int sampleRate, int totalSamples) {
    mvamprg = new VampAnalyser(m_pConfigAVR);
    m_bPass = false;
    bool bAnalyserEnabled = (bool)m_pConfigAVR->getValueString(ConfigKey("[ReplayGain]","ReplayGainAnalyserEnabled")).toInt();
    float fReplayGain = tio->getReplayGain();
    if(totalSamples == 0 || fReplayGain != 0 || !bAnalyserEnabled) {
        //qDebug() << "Replaygain Analyser will not start.";
        //if (fReplayGain != 0 ) qDebug() << "Found a ReplayGain value of " << 20*log10(fReplayGain) << "dB for track :" <<(tio->getFilename());
        return 0;
   }
    int fastAnalyse=1;
    m_bPass = mvamprg->Init("libmixxxminimal","replaygain:0",sampleRate, totalSamples,fastAnalyse);
    if (!m_bPass)
        qDebug() << "Failed to init Vamp Replay Gain Analyser";
}

void AnalyserGainVamp::process(const CSAMPLE *pIn, const int iLen) {
    if (!m_bPass) {
        return;
    }
    m_bPass = mvamprg->Process(pIn, iLen);
}

void AnalyserGainVamp::finalise(TrackPointer tio) {
    if (!m_bPass) {
        return;
    }
    QVector<double> values;
    values = mvamprg->GetFirstValuesVector();
    if (!values.isEmpty()) {
        //qDebug()<<"Found a ReplayGain value of"<<pow(10,values[0]/20);
        float fReplayGain_Result = pow(10,values[0]/20);
        tio->setReplayGain(fReplayGain_Result);
    }
    values.clear();
    m_bPass = mvamprg->End();
    if(m_bPass)
        qDebug()<<"Optimal Gain detection complete";
    else
        qDebug()<<"Error in Optimal Gain detection";

    delete mvamprg;
    mvamprg = NULL;
}
