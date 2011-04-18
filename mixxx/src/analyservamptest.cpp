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
#include "analyservamptest.h"
#include "track/beatmatrix.h"
#include "track/beatfactory.h"

AnalyserVampTest::AnalyserVampTest(ConfigObject<ConfigValue> *_config) {
    m_pConfigAVT = _config;
    m_bPass = 0;

    mvamp = new VampAnalyser();

}

AnalyserVampTest::~AnalyserVampTest(){
    delete mvamp;
}
void AnalyserVampTest::initialise(TrackPointer tio, int sampleRate,
        int totalSamples) {
    //usage mvamp->Init(plugin key, output number, samplerate, totalsamples);
    //tested beat tracking features with vamp-plugins:
    //"vamp-aubio:aubiotempo"(GPLed)
    //"qm-vamp-plugins:qm-barbeattracker" (now released under GPL)
    //"qm-vamp-plugins:qm-tempotracker" (now released under GPL)
    m_bPass = mvamp->Init("qm-subset:qm-barbeattracker",0,sampleRate, totalSamples);
    if (!m_bPass)
        qDebug() << "Failed to init";

    //   m_iStartTime = clock();
}

void AnalyserVampTest::process(const CSAMPLE *pIn, const int iLen) {
    if(!m_bPass) return;
    m_bPass = mvamp->Process(pIn, iLen);
}

void AnalyserVampTest::finalise(TrackPointer tio) {
    if(!m_bPass) return;

   QVector <double> beats;
    if(mvamp->GetInitFramesVector(&beats)){
        BeatsPointer pBeats = BeatFactory::makeBeatMatrix(tio, beats);
        tio->setBeats(pBeats);
        tio->setBpm(pBeats->getBpm());
    }
    m_bPass = mvamp->End();
    beats.clear();
    if(m_bPass) qDebug()<<"Beat Calculation complete";
    //m_iStartTime = clock() - m_iStartTime;
}

