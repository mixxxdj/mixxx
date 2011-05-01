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
#include "track/beatmatrix.h"
#include "track/beatfactory.h"
#include "analyserbeats.h"

AnalyserBeats::AnalyserBeats(ConfigObject<ConfigValue> *_config) {
    m_pConfigAVT = _config;
    m_bPass = 0;



}

AnalyserBeats::~AnalyserBeats(){

}
void AnalyserBeats::initialise(TrackPointer tio, int sampleRate,
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
    //"libmixxxminimal:qm-barbeattracker" with output 0 to activate Queens Mary plugin beat detection

    m_bPass = false;
    mvamp = new VampAnalyser();
    QString library = m_pConfigAVT->getValueString(ConfigKey("[Vamp]","AnalyserBeatLibrary"));
    QString pluginID = m_pConfigAVT->getValueString(ConfigKey("[Vamp]","AnalyserBeatPluginID"));
    //usage               "plugin key"                         output  samplerate  totalsamples
    m_bPass = mvamp->Init(library, pluginID, sampleRate, totalSamples);
    if (!m_bPass){
        qDebug() << "Cannot initialize vamp plugin";
        return;
    }
    //   m_iStartTime = clock();
}

void AnalyserBeats::process(const CSAMPLE *pIn, const int iLen) {
    if(!m_bPass) return;
    m_bPass = mvamp->Process(pIn, iLen);
}

void AnalyserBeats::finalise(TrackPointer tio) {
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
