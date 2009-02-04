#ifndef ANALYSER_BPM_H
#define ANALYSER_BPM_H

#include "analyser.h"
#include "configobject.h"
#include "bpm/bpmdetect.h"


#define BPM_NUM_SAMPLES 32768 //16384; //8192;                         

class BpmDetect;

class AnalyserBPM : public Analyser {

public:
	AnalyserBPM(ConfigObject<ConfigValue> *_config);
	void initialise(TrackInfoObject* tio, int sampleRate, int totalSamples);
	void process(const CSAMPLE *pIn, const int iLen);
	void finalise(TrackInfoObject* tio);

private:
    ConfigObject<ConfigValue> *m_pConfig;
    BpmDetect *m_pDetector;
    int m_iMinBpm, m_iMaxBpm;
    bool m_bProcessEntireSong;

    //this is the buffer that process() uses; we create it here to avoid:
    // a) smashing the function stack
    // b) using malloc, which is slow for a tight-loop member like process()
    soundtouch::SAMPLETYPE samples[BPM_NUM_SAMPLES];

};

#endif
