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

    //"pluginID"
    //tested beat tracking features with vamp-plugins:
    //"vamp-aubio:aubiotempo"(GPLed)
    //"qm-vamp-plugins:qm-barbeattracker" (now released under GPL)
    //"qm-vamp-plugins:qm-tempotracker" (now released under GPL)
    mvamp = new VampAnalyser("qm-vamp-plugins:qm-barbeattracker");

}

AnalyserVampTest::~AnalyserVampTest(){
    delete mvamp;
}
void AnalyserVampTest::initialise(TrackPointer tio, int sampleRate,
        int totalSamples) {
    m_bPass = mvamp->Init(sampleRate, totalSamples);
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

    VampPluginEventList collect = mvamp->GetResults();
    float bpm = 0;
    int count = 0;
//    BeatMatrix* BeatMat = new BeatMatrix(tio);
//    for (int i = 0; i < collect.size(); ++i) {
//        if(!(collect[i]).isFromOutput)
//                BeatMat->addBeat((collect[i]).StartingFrame * 2);
//        if((collect[i]).isFromOutput == 2){//This works only with "qm-vamp-plugins:qm-tempotracker"
//            bpm += collect[i].Values[0];
//            count++;
//        }
//    }
//    if(count)qDebug()<<"bpm: "<< bpm/count;
    QVector <double> results;
    for (int i = 0; i < collect.size(); ++i) {
           if(!(collect[i]).isFromOutput)
               results << ((collect[i]).StartingFrame * 2);
    }
    BeatsPointer pBeats = BeatFactory::makeBeatMatrix(tio, results);
    tio->setBeats(pBeats);
    m_bPass = mvamp->End();
    results.clear();
    if(!collect.isEmpty()) collect.clear();
    //delete BeatMat;
    //m_iStartTime = clock() - m_iStartTime;
}

