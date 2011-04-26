/*
 * analyservamptest.cpp
 *
 *  Created on: 16/mar/2011
 *      Author: vittorio
 */

#include <QtDebug>
#include <QVector>
#include <QString>
#include <time.h>

#include "trackinfoobject.h"
#include "analyservamptest.h"
#include "track/beatmatrix.h"
#include "track/beatfactory.h"

AnalyserVampTest::AnalyserVampTest(ConfigObject<ConfigValue> *_config) {
    m_pConfigAVT = _config;
    m_bPass = 0;



}

AnalyserVampTest::~AnalyserVampTest(){

}
void AnalyserVampTest::initialise(TrackPointer tio, int sampleRate,
        int totalSamples) {

//    if(tio->getBpm() != 0)
//        return;

    //tested beat tracking features with vamp-plugins:
    //"vamp-aubio:aubiotempo"(GPLed)
    //"qm-vamp-plugins:qm-barbeattracker" (now released under GPL)
    //"qm-vamp-plugins:qm-tempotracker" (now released under GPL)
    //
    //Choose:
    //"libmixxxminimal:mixxxbeatdetection" with output 0 to activate standard Mixxx bpm detection via soundtouch
    //"libmixxxminimal:qm-barbeattracker" with output 0 to activate Queens Mary beat detection

    mvamp = new VampAnalyser();
    //usage               "plugin key"                         output  samplerate  totalsamples
    m_bPass = mvamp->Init("libmixxxminimal:qm-barbeattracker", 0     , sampleRate, totalSamples);
    if (!m_bPass){
        qDebug() << "Cannot initialise vamp plugin";
        return;
    }
    //   m_iStartTime = clock();
}

void AnalyserVampTest::process(const CSAMPLE *pIn, const int iLen) {
    if(!m_bPass) return;
    m_bPass = mvamp->Process(pIn, iLen);
}

void AnalyserVampTest::finalise(TrackPointer tio) {
    if(!m_bPass) return;

   QVector <double> beats;
   beats = mvamp->GetInitFramesVector();
    if(!beats.isEmpty()){
        BeatsPointer pBeats = BeatFactory::makeBeatMatrix(tio, beats);
        tio->setBeats(pBeats);
        tio->setBpm(pBeats->getBpm());
    }
    m_bPass = mvamp->End();
    beats.clear();
    if(m_bPass) qDebug()<<"Beat Calculation complete";
    //m_iStartTime = clock() - m_iStartTime;
    delete mvamp;
}

