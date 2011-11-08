
#include <QtDebug>

#include "BPMDetect.h"
#include "trackinfoobject.h"
#include "track/beatgrid.h"
#include "track/beatfactory.h"
#include "analyserbpm.h"


AnalyserBPM::AnalyserBPM(ConfigObject<ConfigValue> *_config) {
    m_pConfig = _config;
    m_pDetector = NULL;
}

void AnalyserBPM::initialise(TrackPointer tio, int sampleRate, int totalSamples) {
    Q_UNUSED(totalSamples);
    m_iMinBpm = m_pConfig->getValueString(ConfigKey("[BPM]","BPMRangeStart")).toInt();
    m_iMaxBpm = m_pConfig->getValueString(ConfigKey("[BPM]","BPMRangeEnd")).toInt();
    m_bProcessEntireSong = (bool)m_pConfig->getValueString(ConfigKey("[BPM]","AnalyzeEntireSong")).toInt();
    // var not used, remove? TODO(bkgood)
    // int defaultrange = m_pConfig->getValueString(ConfigKey("[BPM]","BPMAboveRangeEnabled")).toInt();
    bool bpmEnabled = (bool)m_pConfig->getValueString(ConfigKey("[BPM]","BPMDetectionEnabled")).toInt();

    // If BPM detection is enabled and the track does not have a BPM already,
    // create a detector.
    if(bpmEnabled && tio->getBpm() <= 0.0) {
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

        // Currently, the BPM is only analyzed if the track has no BPM. This
        // means we don't have to worry that the track already has an existing
        // BeatGrid.
        BeatsPointer pBeats = BeatFactory::makeBeatGrid(tio, newbpm, 0.0f);
        tio->setBeats(pBeats);

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
