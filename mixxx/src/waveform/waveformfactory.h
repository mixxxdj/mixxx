#ifndef WAVEFORMFACTORY_H
#define WAVEFORMFACTORY_H

#include "library/dao/analysisdao.h"

class Waveform;

#define WAVEFORM_2_VERSION "Waveform-2.0"
#define WAVEFORMSUMMARY_2_VERSION "WaveformSummary-2.0"
#define WAVEFORM_2_DESCRIPTION "Waveform 2.0"
#define WAVEFORMSUMMARY_2_DESCRIPTION "WaveformSummary 2.0"

#define WAVEFORM_3_VERSION "Waveform-3.0"
#define WAVEFORMSUMMARY_3_VERSION "WaveformSummary-3.0"
#define WAVEFORM_3_DESCRIPTION "Waveform 3.0"
#define WAVEFORMSUMMARY_3_DESCRIPTION "WaveformSummary 3.0"

class WaveformFactory {
  public:
    static Waveform* loadWaveformFromAnalysis(
        TrackPointer pTrack, const AnalysisDao::AnalysisInfo& analysis);
    static QString getPreferredWaveformVersion();
    static QString getPreferredWaveformDescription();
    static QString getPreferredWaveformSummaryVersion();
    static QString getPreferredWaveformSummaryDescription();
};

#endif /* WAVEFORMFACTORY_H */
