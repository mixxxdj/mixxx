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

static bool sDebug = false;

AnalyserBeats::AnalyserBeats(ConfigObject<ConfigValue> *_config) {
    m_pConfigAVT = _config;
    m_bPass = 0;
    m_iSampleRate = 0;
    m_iTotalSamples = 0;
}

AnalyserBeats::~AnalyserBeats(){
}

void AnalyserBeats::initialise(TrackPointer tio, int sampleRate, int totalSamples) {
    Q_UNUSED(sampleRate);
    m_iMinBpm = m_pConfigAVT->getValueString(ConfigKey("[BPM]","BPMRangeStart")).toInt();
    m_iMaxBpm = m_pConfigAVT->getValueString(ConfigKey("[BPM]","BPMRangeEnd")).toInt();
    int allow_above =m_pConfigAVT->getValueString(ConfigKey("[BPM]","BPMAboveRangeEnabled")).toInt();
    if(allow_above){
        m_iMinBpm = 0;
        m_iMaxBpm = 9999;
    }
    QString library = m_pConfigAVT->getValueString(ConfigKey("[Vamp]","AnalyserBeatLibrary"));
    QString pluginID = m_pConfigAVT->getValueString(ConfigKey("[Vamp]","AnalyserBeatPluginID"));
    /*
     *At first start config for QM and Vamp does not exist --> set default
     */
    if(library.isEmpty() || library.isNull())
        library = "libmixxxminimal";
    if(pluginID.isEmpty() || pluginID.isNull())
        pluginID="qm-tempotracker:0";

    m_sSubver = "";
    m_iSampleRate = tio->getSampleRate();
    m_iTotalSamples = totalSamples;
    m_bPass = false;
    m_bPass = static_cast<bool> (m_pConfigAVT->getValueString(ConfigKey("[Vamp]","AnalyserBeatEnabled")).toInt());
    qDebug()<<"Beat calculation started";
    qDebug()<<"Beat calculation uses "<< pluginID;
    QString correction;
    m_bDisableBeatCorrection = !static_cast<bool>(m_pConfigAVT->
                                                   getValueString(ConfigKey("[Vamp]","AnalyserBeatFixedTempo")).toInt());

    if(!m_bDisableBeatCorrection){
    m_bEnableOffsetCorrection = static_cast<bool> (m_pConfigAVT->
                                                   getValueString(ConfigKey("[Vamp]","AnalyserBeatOffset")).toInt());
    if(m_bEnableOffsetCorrection)
        correction = "offset";
    else
        correction = "const";
    }
    else
        correction = "none";
    QString pluginname = pluginID;
    pluginname.replace(QString(":"),QString("_output="));
    m_sSubver.append(QString("_plugin=%1_beats_correction=%2").arg(pluginname,correction));
    BeatsPointer pBeats = tio->getBeats();
    if(pBeats)
        m_bPass = ! pBeats->getVersion().contains(QString("_plugin=%1_beats_correction=%2").arg(pluginname,correction)) ;
    if(!m_bPass){
        qDebug()<<"Beat calculation will not start";
        return;
    }
      //qDebug() << "Init Vamp Beat tracker with samplerate " << tio->getSampleRate() << " " << sampleRate;
      //qDebug()<<"SubVersion is "<<m_sSubver;
      mvamp = new VampAnalyser();
      m_bPass = mvamp->Init(library, pluginID, m_iSampleRate, totalSamples);
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
       m_dBpm = BeatTools::calculateBpm(beats, m_iSampleRate, m_iMinBpm, m_iMaxBpm);
       BeatsPointer pBeats = BeatFactory::makeBeatMap(tio, correctedBeats(beats,m_bDisableBeatCorrection),m_sSubver);
       tio->setBeats(pBeats);
       tio->setBpm(m_dBpm);
   }
   else{
       qDebug() << "Could not detect beat positions from Vamp.";
   }

    beats.clear();
    m_sSubver="";
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


    QVector <double> corrbeats;
    double BpmFrame = (60.0 * m_iSampleRate / m_dBpm);
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

    double offset = 0;

    if(m_bEnableOffsetCorrection){
        qDebug()<<"Calculating best offset";

        offset = BeatTools::calculateOffset(rawbeats, corrbeats, m_iSampleRate, m_iMinBpm,  m_iMaxBpm);
    }

        corrbeats.clear();
        double FirstFrame = offset + rawbeats.at(0);
        while (FirstFrame < 0)
            FirstFrame += BpmFrame;
        while (FirstFrame > BpmFrame)
            FirstFrame -= BpmFrame;
        i = floor(FirstFrame + 0.5);
        if(sDebug){
            qDebug()<<"First Frame is at " << i;
            qDebug()<<"It was at " << rawbeats.at(0);
        }
        while(i < m_iTotalSamples){
            corrbeats << i;
            i += BpmFrame;
        }

    return corrbeats;

}

