
#include <QtDebug>

#include "BPMDetect.h"
#include "trackinfoobject.h"
#include "analyserbpm.h"


AnalyserBPM::AnalyserBPM(ConfigObject<ConfigValue> *_config) {
    m_pConfig = _config;
    m_pDetector = NULL;
}

void AnalyserBPM::initialise(TrackPointer tio, int sampleRate, int totalSamples) {
    m_iMinBpm = m_pConfig->getValueString(ConfigKey("[BPM]","BPMRangeStart")).toInt();
    m_iMaxBpm = m_pConfig->getValueString(ConfigKey("[BPM]","BPMRangeEnd")).toInt();
    m_bProcessEntireSong = (bool)m_pConfig->getValueString(ConfigKey("[BPM]","AnalyzeEntireSong")).toInt();
    int defaultrange = m_pConfig->getValueString(ConfigKey("[BPM]","BPMAboveRangeEnabled")).toInt();
    bool bpmEnabled = (bool)m_pConfig->getValueString(ConfigKey("[BPM]","BPMDetectionEnabled")).toInt();

    // If BPM detection is not enabled, or the track already has BPM detection done.
    if(bpmEnabled &&
       tio->getBpm() == 0.) {
        // All SoundSource's return stereo data, no matter the real file's type
        m_pDetector = new soundtouch::BPMDetect(2, sampleRate);
        //m_pDetector = new BPMDetect(tio->getChannels(), sampleRate);
        //                                    defaultrange ? MIN_BPM : m_iMinBpm,
        //                                    defaultrange ? MAX_BPM : m_iMaxBpm);
    }
}



void AnalyserBPM::process(const CSAMPLE *pIn, const int iLen) {

    // Check if BPM detection is enabled
    if(m_pDetector == NULL) {
        return;
    }
    //qDebug() << "AnalyserBPM::process() processing " << iLen << " samples";

    m_pDetector->inputSamples(pIn, iLen/2);

}

float AnalyserBPM::correctBPM( float BPM, int min, int max, int aboveRange) {
    //qDebug() << "BPM range is" << min << "to" << max;
    if ( BPM == 0 ) return BPM;

    if (aboveRange == 0) {
        if( BPM*2 < max ) BPM *= 2;
        while ( BPM > max ) BPM /= 2;
    }
    while ( BPM < min ) BPM *= 2;

    return BPM;
}

void AnalyserBPM::finalise(TrackPointer tio) {
    // Check if BPM detection is enabled
    if(m_pDetector == NULL) {
        return;
    }

    float bpm = m_pDetector->getBpm();
    if(bpm != 0) {
        // Shift it by 2's until it is in the desired range
        float newbpm = correctBPM(bpm, m_iMinBpm, m_iMaxBpm, m_pConfig->getValueString(ConfigKey("[BPM]","BPMAboveRangeEnabled")).toInt());

        tio->setBpm(newbpm);
        tio->setBpmConfirm();
        //if(pBpmReceiver) {
        //pBpmReceiver->setComplete(tio, false, bpm);
        //}
        //qDebug() << "AnalyserBPM BPM detection successful for" << tio->getFilename();
        //qDebug() << "AnalyserBPM BPM is " << newbpm << " (raw: " << bpm << ")";
    } else {
        //qDebug() << "AnalyserBPM BPM detection failed, setting to 0.";
    }

    // Cleanup the BPM detector
    delete m_pDetector;
    m_pDetector = NULL;

}
