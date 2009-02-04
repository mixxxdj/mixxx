
#include <QtDebug>


#include "trackinfoobject.h"
#include "analyserbpm.h"
#include "bpm/bpmdetect.h"

AnalyserBPM::AnalyserBPM(ConfigObject<ConfigValue> *_config) {
    m_pConfig = _config;
    m_pDetector = NULL;
}

void AnalyserBPM::initialise(TrackInfoObject* tio, int sampleRate, int totalSamples) {
    m_iMinBpm = m_pConfig->getValueString(ConfigKey("[BPM]","BPMRangeStart")).toInt();
    m_iMaxBpm = m_pConfig->getValueString(ConfigKey("[BPM]","BPMRangeEnd")).toInt();
    m_bProcessEntireSong = (bool)m_pConfig->getValueString(ConfigKey("[BPM]","AnalyzeEntireSong")).toInt();
    int defaultrange = m_pConfig->getValueString(ConfigKey("[BPM]","BPMAboveRangeEnabled")).toInt();
    bool bpmEnabled = (bool)m_pConfig->getValueString(ConfigKey("[BPM]","BPMDetectionEnabled")).toInt();

    // If BPM detection is not enabled, or the track already has BPM detection done.
    if(bpmEnabled &&
       tio->getBpmConfirm() == false &&
       tio->getBpm() == 0.) {
        m_pDetector = new BpmDetect(tio->getChannels(), sampleRate,
                                    defaultrange ? MIN_BPM : m_iMinBpm,
                                    defaultrange ? MAX_BPM : m_iMaxBpm);
    }
}



void AnalyserBPM::process(const CSAMPLE *pIn, const int iLen) {

    // Check if BPM detection is enabled
    if(m_pDetector == NULL) {
        return;
    }
    //qDebug() << "AnalyserBPM::process() processing " << iLen << " samples";

    
    int cur = 0;
    int remaining = iLen;
    while(remaining > 0) {
        int read = math_min(remaining, BPM_NUM_SAMPLES);
        memcpy(samples, &pIn[cur], sizeof(float)*read);
        /*for(int i=0; i<read; i++) {
            samples[i] = pIn[cur+i];
            }*/
        m_pDetector->inputSamples(samples, read/2);
        cur += read;
        remaining-=read;
    }
}

void AnalyserBPM::finalise(TrackInfoObject *tio) {
    // Check if BPM detection is enabled
    if(m_pDetector == NULL) {
        return;
    }
    
    float bpm = m_pDetector->getBpm();
    if(bpm != 0) {
        bpm = BpmDetect::correctBPM(bpm, m_iMinBpm, m_iMaxBpm);
        tio->setBpm(bpm);
        tio->setBpmConfirm();
        //if(pBpmReceiver) {
        //pBpmReceiver->setComplete(tio, false, bpm);
        //}
        qDebug() << "AnalyserBPM BPM detection successful for" << tio->getFilename();
        qDebug() << "AnalyserBPM BPM is " << bpm;
    } else {
        qDebug() << "AnalyserBPM BPM detection failed, setting to 0.";
    }

    // Cleanup the BPM detector
    delete m_pDetector;
    m_pDetector = NULL;
} 
