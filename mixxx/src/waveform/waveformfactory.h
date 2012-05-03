#ifndef WAVEFORMFACTORY_H
#define WAVEFORMFACTORY_H

#include "library/dao/analysisdao.h"

class Waveform;

class WaveformFactory {
  public:
    static Waveform* loadWaveformFromAnalysis(
        TrackPointer pTrack, const AnalysisDao::AnalysisInfo& analysis);
};

#endif /* WAVEFORMFACTORY_H */
