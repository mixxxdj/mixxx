#ifndef ANALYSER_BPM_H
#define ANALYSER_BPM_H

#include "BPMDetect.h"

#include "analyser.h"
#include "configobject.h"


class AnalyserBPM : public Analyser {
  public:
    AnalyserBPM(ConfigObject<ConfigValue> *_config);
    bool initialise(TrackPointer tio, int sampleRate, int totalSamples);
    void process(const CSAMPLE *pIn, const int iLen);
    void cleanup(TrackPointer tio);
    void finalise(TrackPointer tio);

  private:
    ConfigObject<ConfigValue> *m_pConfig;
    soundtouch::BPMDetect *m_pDetector;
    int m_iMinBpm, m_iMaxBpm;
    bool m_bProcessEntireSong;
};

#endif
