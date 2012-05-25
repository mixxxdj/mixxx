#ifndef WAVEFORMFACTORY_H
#define WAVEFORMFACTORY_H

#include "library/dao/analysisdao.h"

class Waveform;

#define WAVEFORM_2_VERSION "Waveform-2.0"
#define WAVEFORMSUMMARY_2_VERSION "WaveformSummary-2.0"

class WaveformFactory {
  public:
    static Waveform* loadWaveformFromAnalysis(
        TrackPointer pTrack, const AnalysisDao::AnalysisInfo& analysis);
    static QString getPreferredWaveformVersion();
    static QString getPreferredWaveformSummaryVersion();
};

#endif /* WAVEFORMFACTORY_H */
