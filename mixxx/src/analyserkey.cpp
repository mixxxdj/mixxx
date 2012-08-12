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
#include <iostream>
#include <stdio.h>

#include "trackinfoobject.h"
#include "analyserkey.h"
//#include "track/beatmatrix.h"
#include "track/beatfactory.h"
#include "track/key_preferences.h"


using namespace std;
static bool sDebug = false;

AnalyserKey::AnalyserKey(ConfigObject<ConfigValue> *_config):m_pConfig(_config) {
    m_bShouldAnalyze = false;

    //"pluginID"
    //tested key detection features with vamp-plugins:
    // qm-vamp-plugins:qm-keydetector (GPLed)
//QString library = m_pConfigAVT->getValueString(ConfigKey("[Vamp]","AnalyserKeyLibrary"));
//QString pluginID = m_pConfigAVT->getValueString(ConfigKey("[Vamp]","AnalyserKeyPluginID"));
//if(library.isEmpty() || library.isNull())
//        library = "libmixxxminimal";
//    if(pluginID.isEmpty() || pluginID.isNull())
//        pluginID="qm-keydetector:0";

// 	mvamp = new VampAnalyser(m_pConfigAVT);

}

AnalyserKey::~AnalyserKey(){
    delete m_pVamp;
}

bool AnalyserKey::initialise(TrackPointer tio, int sampleRate,
        int totalSamples) {
    m_bShouldAnalyze=false;
    if (totalSamples == 0)
        return false;

    QString library = m_pConfig->getValueString(ConfigKey("[Vamp]","AnalyserKeyLibrary"));
    QString pluginID = m_pConfig->getValueString(ConfigKey("[Vamp]","AnalyserKeyPluginID"));
    if(library.isEmpty() || library.isNull())
      library = "libmixxxminimal";
    if(pluginID.isEmpty() || pluginID.isNull())
      pluginID="qm-keydetector:0";

    m_pVamp = new VampAnalyser(m_pConfig);

     m_bPreferencesfastAnalysisEnabled = static_cast<bool>(
         m_pConfig->getValueString(
         ConfigKey(KEY_CONFIG_KEY, KEY_FAST_ANALYSIS)).toInt());

    m_bPreferencesfirstLastEnabled = static_cast<bool>(
         m_pConfig->getValueString(
         ConfigKey(KEY_CONFIG_KEY, KEY_FIRST_LAST)).toInt());

    m_bPreferencesreanalyzeEnabled = static_cast<bool>(
         m_pConfig->getValueString(
         ConfigKey(KEY_CONFIG_KEY, KEY_REANALYZE)).toInt());

    m_bPreferencesskipRelevantEnabled = static_cast<bool>(
         m_pConfig->getValueString(
         ConfigKey(KEY_CONFIG_KEY, KEY_SKIP_RELEVANT)).toInt());

    m_bPreferencesKeyDetectionEnabled = static_cast<bool>(
         m_pConfig->getValueString(
         ConfigKey(KEY_CONFIG_KEY, KEY_DETECTION_ENABLED)).toInt());


    if (!m_bPreferencesKeyDetectionEnabled) {
        qDebug() << "Key detection is deactivated";
        m_bShouldAnalyze = false;
        return false;
    }
    if (m_bPreferencesskipRelevantEnabled && (tio->getKey() != 0)){
        m_bShouldAnalyze = false;
        return false;
    }

    m_bShouldAnalyze=true;
 //qDebug()<<m_bPreferencesskipRelevantEnabled<<"asasda";
//KeyFinder::Parameters p;

/* keyfinderAudio.setFrameRate(sampleRate);
 keyfinderAudio.setChannels(1);
 outSamples = 0;
 totSamples = totalSamples/2; //since libkeyfinder only uses one channel
 keyfinderAudio.addToSampleCount(totSamples);*/

 //int fastAnalyse=1;
   // m_bPass = mvamp->Init(library, pluginID,sampleRate, totalSamples, m_bPreferencesfastAnalysisEnabled);
     m_bShouldAnalyze = m_pVamp->Init(library, pluginID,sampleRate, totalSamples, m_bPreferencesfastAnalysisEnabled);
     m_pVamp->SelectOutput(2);

     if (!m_bShouldAnalyze)
     {
         delete m_pVamp;
         m_pVamp = NULL;
     }
     return m_bShouldAnalyze;

     /*channels = 2; //for libsamplerate
    converter = SRC_SINC_BEST_QUALITY;
    if ((src_state = src_new (converter, channels, &error)) == NULL)
        qDebug() << "Error : src_new() failed : "<< src_strerror (error);
    downsamplingRatio = 4410.0/44100.0; //downsampling ratio
    //qDebug() << "Failed to init key!";


    //   m_iStartTime = clock();*/
}

void AnalyserKey::process(const CSAMPLE *pIn, const int iLen) {
    //if(!m_bPass) return;
    //qDebug()<< "key detecasdasdt";
    //qDebug()<<"key 1";
    if (!m_bShouldAnalyze || m_pVamp == NULL)
        return;

    m_bShouldAnalyze = m_pVamp->Process(pIn, iLen);

    if (!m_bShouldAnalyze) {
        delete m_pVamp;
        m_pVamp = NULL;
    }

    //m_bPass = m_pVamp->Process(pIn, iLen);
    //qDebug()<< "key detecasdasdt";
 /*   int iIN=0;
    //SRC_DATA *src_in;
    //long k = iLen;
    //qDebug()<< "key detecasdasdt 1";
    //src_in->input_frames = iLen ;
    src_in.data_in = pIn;
    //qDebug()<< "key detecasdasdt asda 2";

    const float *output = new float[iLen];
    //qDebug()<< "key detecasdasdt 3";
    src_in.data_out = output;
    //qDebug()<< "key detecasdasdt asda 3";
    src_in.output_frames = iLen;//iLen*downsamplingRatio;
    //qDebug()<< "key detecasdasdt asda 4";
    src_in.src_ratio = downsamplingRatio;
    src_in.end_of_input = 0;
    src_in.input_frames = iLen+5 ;
    //qDebug()<< "key ";
//    if (error = src_process(src_state,&src_in))
       // qDebug() << "Error  : "<< src_strerror (error);
    //qDebug()<<src_in.src_ratio;

    while (iIN < iLen / 10 && outSamples<totSamples/10) {
        keyfinderAudio.setSample(outSamples,output[2*iIN]);
        iIN++;
        outSamples++;
    }
    delete[] output;*/


}

void AnalyserKey::finalise(TrackPointer tio) {
    //if(!m_bPass) return;

    if (!m_bShouldAnalyze || m_pVamp == NULL) {
        return;
    }

    //enum {'C','C#','D','D#','E','F','F#','G','G#','A','A#','B','c','c#','d','d#','e','f','f#','g','g#','a','a#','b'}keys;
    static const char *keys[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B","c","c#","d","d#","e","f","f#","g","g#","a","a#","b"};
   // static const char *keyfind[] = {"A","a","A#","a#","B","b","C","c","C#","c#","D","d","D#","d#","E","e","F","f","F#","f#","G","g","G#","g#","SILENCE"};
    //keyfinderParams.setHopSize(8192);
   // keyfinderResult = keyfinderKey.findKey(keyfinderAudio,keyfinderParams);
    //qDebug()<<"key 2";
  //  qDebug()<<"libkeyfinder result"<< keyfind[keyfinderResult.globalKeyEstimate];
    //if(mvamp->GetInitFramesVector(&m_frames)){
      //  if(mvamp->GetLastValuesVector(&m_keys))
    bool success = m_pVamp->End();
    qDebug() << "Beat Calculation" << (success ? "complete" : "failed");

    m_frames= m_pVamp->GetInitFramesVector();
    m_keys= m_pVamp->GetLastValuesVector();

    vector<float> key_times(24,0);
    map<float,int> key_time;
    map<float,int>::reverse_iterator it;
    //qDebug()<<""<<tio->getKey();
    //qDebug()<<"key 3";
    //qDebug()<<""<<tio->getKey();
    for (int i=0;i<(m_keys.size()-1);i++){
         key_times[m_keys[i]-1] = key_times[m_keys[i]-1]+ (m_frames[i+1]-m_frames[i]) ;
    }

    for(int i=0;i<24;i++)
        key_time.insert(pair<float,int>(key_times[i],i));
    it = key_time.rbegin();
    qDebug() << keys[(*it).second]<<" at "<<(*it).first;
    it++;
    qDebug() << keys[(*it).second]<<"  "<<(*it).first;
    it--;
    qDebug()<<tio->getKey();
    //qDebug()<<m_bPreferencesKeyDetectionEnabled<<m_bPreferenceswriteTagsEnabled<<m_bPreferencesfirstLastEnabled<<m_bPreferencesreanalyzeEnabled<<m_bPreferencesskipRelevantEnabled;
   // qDebug()<<m_bPreferenceswriteTagsEnabled;
    //qDebug()<<"key 4";
    tio->setKey(keys[(*it).second]);
    //tio->setKey("a");
    //qDebug()<<"key 5";
    //tio->setKey(keyfind[keyfinderResult.globalKeyEstimate]);
            //key_time.insert(pair<float,int>(m_frames[i+1]-m_frames[i],m_keys[i]))
                  //  mymap.insert ( pair<char,int>('a',100) );
        //}
   // for (int i=0; i<m_frames.size(); i++){
               // qDebug()<<" "<<m_keys[i]<<" at frame "<<m_frames[i];
    //}

    m_bShouldAnalyze = m_pVamp->End();
    m_frames.clear();
    m_keys.clear();
    //src_delete(src_state);
   // qDebug() << "Key detection ";
    //m_iStartTime = clock() - m_iStartTime;
}

void AnalyserKey::cleanup(TrackPointer tio) {
    Q_UNUSED(tio);
    if (!m_bShouldAnalyze) {
        return;
    }
    delete m_pVamp;
    m_pVamp = NULL;
}
