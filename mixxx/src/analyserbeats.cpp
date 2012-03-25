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
    m_bPass = false;
    m_iSampleRate = 0;
    m_iTotalSamples = 0;
}

AnalyserBeats::~AnalyserBeats(){
}

void AnalyserBeats::initialise(TrackPointer tio, int sampleRate, int totalSamples) {
    m_bPass = false;
    if (totalSamples==0)
        return;
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
    m_iSampleRate = sampleRate;
    m_iTotalSamples = totalSamples;
    bool BpmLock = tio->hasBpmLock();
    if(BpmLock){//Note that m_bPass had been set to false
              qDebug()<<"Track is BpmLocked: Beat calculation will not start";
              return;
          }
    m_bPass = static_cast<bool> (m_pConfigAVT->getValueString(ConfigKey("[Vamp]","AnalyserBeatEnabled")).toInt());

    if(!m_bPass)
    {
        qDebug()<<"Beat calculation is deactivated";
        return;
    }

    // If the track has a BeatGrid rather than a BeatMap return unless it is specified in the preferences
    if(tio->getBeats() != NULL)
    {
        if(tio->getBeats()->getVersion() == BEAT_GRID_VERSION)
        {
            m_bPass = static_cast<bool>(m_pConfigAVT->getValueString(ConfigKey("[Vamp]","ReanalyzeOldBPM")).toInt());
            if(!m_bPass)
            {
                qDebug()<<"Beat calculation skips analyzing because the track has a BPM computed by a previous Mixxx version.";
                return;
            }
        }
    }

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

    //Check if BPM algorithm has changed
    QString pluginname = pluginID;
    pluginname.replace(QString(":"),QString("_output="));
    m_sSubver.append(QString("plugin=%1_beats_correction=%2").arg(pluginname,correction));
    BeatsPointer pBeats = tio->getBeats();
    QString bpmpluginkey = tio->getBpmPluginKey();

    if(pBeats)
        m_bPass = !bpmpluginkey.contains(QString("plugin=%1_beats_correction=%2").arg(pluginname,correction));

    if(!m_bPass)
    {
        qDebug()<<"Beat calculation will not start";
        return;
    }

    mvamp = new VampAnalyser(m_pConfigAVT);
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
       BeatsPointer pBeats = BeatFactory::makeBeatMap(tio, correctedBeats(beats,m_bDisableBeatCorrection));
       tio->setBeats(pBeats);
       tio->setBpmPluginKey(m_sSubver);
       tio->setBpm(m_dBpm);
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


    QVector <double> corrbeats;
    double BpmFrame = (60.0 * m_iSampleRate / m_dBpm);
    /*
     * We start building a grid:
     */
    double i = BeatTools::findFirstCorrectBeat(rawbeats,m_iSampleRate,m_dBpm);
    while(i <= m_iTotalSamples){
           corrbeats << i;
           i += BpmFrame;
       }
    if (rawbeats.size()==1 || corrbeats.size()==1 )
            return corrbeats;
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
        double FirstFrame = offset + BeatTools::findFirstCorrectBeat(rawbeats,m_iSampleRate,m_dBpm);
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

