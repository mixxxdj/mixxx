/*
 * analyservamptest.cpp
 *
 *  Created on: 16/mar/2011
 *      Author: vittorio
 */

#include <QtDebug>
#include <QList>
#include <QVector>
#include <QString>
#include <time.h>
#include <math.h>

#include "trackinfoobject.h"
#include "analyservampkeytest.h"
#include "track/beatmatrix.h"
#include "track/beatfactory.h"

AnalyserVampKeyTest::AnalyserVampKeyTest(ConfigObject<ConfigValue> *_config) {
    m_pConfigAVT = _config;
    m_bPass = 0;

    //"pluginID"
    //tested key detection features with vamp-plugins:
    // qm-vamp-plugins:qm-keydetector (GPLed)
    mvamp = new VampAnalyser("qm-vamp-plugins:qm-keydetector");

}

AnalyserVampKeyTest::~AnalyserVampKeyTest(){
    delete mvamp;
}
void AnalyserVampKeyTest::initialise(TrackPointer tio, int sampleRate,
        int totalSamples) {
    m_bPass = mvamp->Init(sampleRate, totalSamples);
    mvamp->SelectOutput(2);
    if (!m_bPass)
        qDebug() << "Failed to init";

    //   m_iStartTime = clock();
}

void AnalyserVampKeyTest::process(const CSAMPLE *pIn, const int iLen) {
    if(!m_bPass) return;
    m_bPass = mvamp->Process(pIn, iLen);

}

void AnalyserVampKeyTest::finalise(TrackPointer tio) {
    if(!m_bPass) return;

    if(mvamp->GetInitFramesVector(&m_frames)){
        if(mvamp->GetLastValuesVector(&m_keys))
            for (int i=0; i<m_frames.size(); i++){
                qDebug()<<"Key changes to "<<m_keys[i]<<" at frame "<<m_frames[i];
            }
    }
    m_bPass = mvamp->End();
    m_frames.clear();
    m_keys.clear();
    qDebug()<<"Key detection complete";
    //m_iStartTime = clock() - m_iStartTime;
}

