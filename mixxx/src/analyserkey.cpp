#include <QtDebug>
#include <QList>
#include <QVector>
#include <QString>
#include <time.h>
#include <math.h>
#include <iostream>
#include <stdio.h>

#include "analyserkey.h"
#include "track/key_preferences.h"
#include "proto/keys.pb.h"
#include "track/keyfactory.h"

using namespace std;
static bool sDebug = false;

using mixxx::track::io::key::ChromaticKey;
using mixxx::track::io::key::ChromaticKey_IsValid;

AnalyserKey::AnalyserKey(ConfigObject<ConfigValue> *_config)
        : m_pConfig(_config),
          m_iTotalSamples(0),
          m_bShouldAnalyze(false) {
}

AnalyserKey::~AnalyserKey(){
    delete m_pVamp;
}

bool AnalyserKey::initialise(TrackPointer tio, int sampleRate, int totalSamples) {
    m_bShouldAnalyze = false;
    m_iSampleRate = sampleRate;
    m_iTotalSamples = totalSamples;
    if (loadStored(tio) || totalSamples == 0) {
        return false;
    }

    QString library = m_pConfig->getValueString(ConfigKey("[Vamp]","AnalyserKeyLibrary"));
    QString pluginID = m_pConfig->getValueString(ConfigKey("[Vamp]","AnalyserKeyPluginID"));

    if(library.isEmpty() || library.isNull())
        library = "libmixxxminimal";

    if(pluginID.isEmpty() || pluginID.isNull())
        pluginID="qm-keydetector:3";
    m_pluginId = pluginID;

    m_pVamp = new VampAnalyser(m_pConfig);

    m_bPreferencesFastAnalysisEnabled = static_cast<bool>(
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

    m_bShouldAnalyze=true;
 //qDebug()<<m_bPreferencesskipRelevantEnabled<<"asasda";
//KeyFinder::Parameters p;

/* keyfinderAudio.setFrameRate(sampleRate);
 keyfinderAudio.setChannels(1);
 outSamples = 0;
 totSamples = totalSamples/2; //since libkeyfinder only uses one channel
 keyfinderAudio.addToSampleCount(totSamples);*/

 //int fastAnalyse=1;
   // m_bPass = mvamp->Init(library, pluginID,sampleRate, totalSamples, m_bPreferencesFastAnalysisEnabled);
     m_bShouldAnalyze = m_pVamp->Init(library, pluginID,sampleRate, totalSamples, m_bPreferencesFastAnalysisEnabled);

     if (m_bShouldAnalyze) {
         // Specific to QM KeyDetect.
         m_pVamp->SelectOutput(2);
     } else {
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

bool AnalyserKey::loadStored(TrackPointer tio) const {
    // TODO(rryan): implement.
    return false;
}

void AnalyserKey::process(const CSAMPLE *pIn, const int iLen) {
    if (!m_bShouldAnalyze || m_pVamp == NULL)
        return;

    m_bShouldAnalyze = m_pVamp->Process(pIn, iLen);

    if (!m_bShouldAnalyze) {
        delete m_pVamp;
        m_pVamp = NULL;
    }
}

void AnalyserKey::finalise(TrackPointer tio) {
    if (!m_bShouldAnalyze || m_pVamp == NULL) {
        return;
    }

    //enum {'C','C#','D','D#','E','F','F#','G','G#','A','A#','B','c','c#','d','d#','e','f','f#','g','g#','a','a#','b'}keys;
    static const char *keyNames[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B","c","c#","d","d#","e","f","f#","g","g#","a","a#","b"};
   // static const char *keyfind[] = {"A","a","A#","a#","B","b","C","c","C#","c#","D","d","D#","d#","E","e","F","f","F#","f#","G","g","G#","g#","SILENCE"};
    //keyfinderParams.setHopSize(8192);
   // keyfinderResult = keyfinderKey.findKey(keyfinderAudio,keyfinderParams);
    //qDebug()<<"key 2";
  //  qDebug()<<"libkeyfinder result"<< keyfind[keyfinderResult.globalKeyEstimate];
    //if(mvamp->GetInitFramesVector(&m_frames)){
      //  if(mvamp->GetLastValuesVector(&m_keys))
    bool success = m_pVamp->End();
    qDebug() << "Key Detection" << (success ? "complete" : "failed");

    m_frames = m_pVamp->GetInitFramesVector();
    m_keys = m_pVamp->GetLastValuesVector();
    delete m_pVamp;
    m_pVamp = NULL;

    if (m_frames.size() == 0 || m_frames.size() != m_keys.size()) {
        qWarning() << "AnalyserKey: Key sequence and list of times do not match.";
        return;
    }

    KeyChangeList key_changes;

    QMap<int, double> key_histogram;
    for (int i = 0; i < m_keys.size(); ++i) {
        const double frames = (i == m_keys.size() - 1) ?
                m_iTotalSamples/2 - m_frames[i] : m_frames[i+1] - m_frames[i];
        key_histogram[m_keys[i]] += frames;

        if (ChromaticKey_IsValid(m_keys[i])) {
            key_changes.push_back(qMakePair(
                static_cast<ChromaticKey>(m_keys[i]), m_frames[i]));
        }
    }

    QHash<QString, QString> extraVersionInfo;
    extraVersionInfo["vamp_plugin_id"] = m_pluginId;
    if (m_bPreferencesFastAnalysisEnabled) {
        extraVersionInfo["fast_analysis"] = "1";
    }

    Keys keys = KeyFactory::makePreferredKeys(
        tio, key_changes, extraVersionInfo,
        m_iSampleRate, m_iTotalSamples);
    tio->setKeys(keys);

    qDebug() << "Key Histogram";
    double max_delta = 0;
    int max_key = 0;
    for (QMap<int, double>::const_iterator it = key_histogram.begin();
         it != key_histogram.end(); ++it) {
        qDebug() << it.key() << ":" << keyNames[it.key()-1] << it.value();
        if (it.value() > max_delta) {
            max_key = it.key();
            max_delta = it.value();
        }
    }

    if (max_key > 0) {
        qDebug() << "Setting key to:" << max_key << keyNames[max_key-1];
        //tio->setKey(keyNames[max_key-1]);
    }
    //tio->setKey(keyfind[keyfinderResult.globalKeyEstimate]);
            //key_time.insert(pair<float,int>(m_frames[i+1]-m_frames[i],m_keys[i]))
                  //  mymap.insert ( pair<char,int>('a',100) );
        //}
   // for (int i=0; i<m_frames.size(); i++){
               // qDebug()<<" "<<m_keys[i]<<" at frame "<<m_frames[i];
    //}

    m_bShouldAnalyze = m_pVamp->End();
    m_iSampleRate = 0;
    m_iTotalSamples = 0;
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
