#ifndef ANALYSER_BPM_H
#define ANALYSER_BPM_H

#include "analyser.h"
#include "configobject.h"

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
    
};

#endif
