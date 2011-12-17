/*
 * analyservamptest.cpp
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
#include "beattools.h"

static bool sDebug = true;
static bool DisableBpmCorrection = true;
static bool DisableOffsetCorrection = true;

// libmixxxminimal:qm-tempotracker
#define VAMP_MIXXX_MINIMAL "libmixxxminimal"
#define VAMP_PLUGIN_BEAT_TRACKER_ID "qm-tempotracker:0"


// libmvamp:marsyas_ibt:0
//#define VAMP_MIXXX_MINIMAL "libmvamp"
//#define VAMP_PLUGIN_BEAT_TRACKER_ID "marsyas_ibt:0"

//beatroot-vamp:beatroot:0

//#define VAMP_MIXXX_MINIMAL "beatroot-vamp"
//#define VAMP_PLUGIN_BEAT_TRACKER_ID "beatroot:0"


AnalyserBeats::AnalyserBeats(ConfigObject<ConfigValue> *_config) {
    m_pConfigAVT = _config;
    m_bPass = 0;
    m_iSampleRate = 0;
    m_iTotalSamples = 0;
}

AnalyserBeats::~AnalyserBeats(){
}

void AnalyserBeats::initialise(TrackPointer tio, int sampleRate, int totalSamples) {
    m_iMinBpm = m_pConfigAVT->getValueString(ConfigKey("[BPM]","BPMRangeStart")).toInt();
    m_iMaxBpm = m_pConfigAVT->getValueString(ConfigKey("[BPM]","BPMRangeEnd")).toInt();

//    if(tio->getBpm() != 0)
//        return;
   // m_iSampleRate = sampleRate;
    m_iSampleRate = tio->getSampleRate();
    m_iTotalSamples = totalSamples;
    qDebug()<<"Beat calculation started";
    m_bPass = false;
    //BeatsPointer pBeats = tio->getBeats();
    //if(pBeats)
    //    m_bPass = !(pBeats->getVersion()).contains("BeatMatrix");
//    if(!m_bPass){
//        qDebug()<<"BeatMatrix already exists: calculation will not start";
//        return;
//    }
      //qDebug() << "Init Vamp Beat tracker with samplerate " << tio->getSampleRate() << " " << sampleRate;
      mvamp = new VampAnalyser();
      m_bPass = mvamp->Init(VAMP_MIXXX_MINIMAL, VAMP_PLUGIN_BEAT_TRACKER_ID, m_iSampleRate, totalSamples);
}

void AnalyserBeats::process(const CSAMPLE *pIn, const int iLen) {
    if(!m_bPass)
        return;
    m_bPass = mvamp->Process(pIn, iLen);
}

void AnalyserBeats::finalise(TrackPointer tio) {
    if(!m_bPass)
        return;

   QVector <double> beats;
   /*
    * Call End() here, because the number of total samples may
    * have been estimated incorrectly.
    */
   m_bPass = mvamp->End(); // This will ensure that beattracking succeeds in a beat list
   beats = mvamp->GetInitFramesVector();

   if(!beats.isEmpty()){

       BeatsPointer pBeats = BeatFactory::makeBeatMap(tio, correctedBeats(beats,DisableBpmCorrection));
       tio->setBeats(pBeats);
       tio->setBpm(pBeats->getBpm());
   }
   else{
       qDebug() << "Could not detect beat positions from Vamp.";
   }

    beats.clear();
    if(m_bPass)
        qDebug()<<"Beat Calculation complete";
    else
        qDebug()<<"Beat Calculation failed";
    delete mvamp;
}

QVector<double> AnalyserBeats::correctedBeats (QVector<double> rawbeats, bool bypass){
    if(bypass)
        return rawbeats;
    /*
     * By default Vamp does not assume a 4/4 signature.
     * This is basically a good property of Vamp, however,
     * it leads to inaccurate beat grids if a 4/4 signature is given.
     * What is the problem? Almost all modern dance music from the last decades
     * refer to 4/4 signatures. Thus, we must 'correct' the beat positions of Vamp
     */
    double corrected_global_bpm = BeatTools::calculateBpm(rawbeats, m_iSampleRate, m_iMinBpm, m_iMaxBpm);

    QVector <double> corrbeats;
    double BpmFrame = (60.0 * m_iSampleRate / corrected_global_bpm);
    /*
     * We start building a grid:
     */
    double i = rawbeats.at(0);
    while(i < m_iTotalSamples){
           corrbeats << i;
           i += BpmFrame;
       }
    /*
     * BeatTools::calculateOffset compares the beats from Vamp and the beats from
     * the beat grid constructed above. See beattools.* for details.
     */
    if(!DisableOffsetCorrection){
        double offset = BeatTools::calculateOffset(rawbeats, corrbeats, m_iSampleRate, m_iMinBpm,  m_iMaxBpm);
        corrbeats.clear();
        double FirstFrame = offset + rawbeats.at(0);
        while (FirstFrame < 0)
            FirstFrame += BpmFrame;
        while (FirstFrame > BpmFrame)
            FirstFrame -= BpmFrame;
        i = FirstFrame;
        if(sDebug)
            qDebug()<<"First Frame is at " << i <<".It was at " << rawbeats.at(0);
        while(i < m_iTotalSamples){
            corrbeats << i;
            i += BpmFrame;
        }
    }
    return corrbeats;

}

