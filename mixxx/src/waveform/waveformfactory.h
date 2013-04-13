#ifndef WAVEFORMFACTORY_H
#define WAVEFORMFACTORY_H

#include "library/dao/analysisdao.h"

class Waveform;

#define WAVEFORM_2_VERSION "Waveform-2.0"
#define WAVEFORMSUMMARY_2_VERSION "WaveformSummary-2.0"
#define WAVEFORM_2_DESCRIPTION "Waveform 2.0"
#define WAVEFORMSUMMARY_2_DESCRIPTION "WaveformSummary 2.0"

// Used in Mixxx 1.11 beta
#define WAVEFORM_3_VERSION "Waveform-3.0"
#define WAVEFORMSUMMARY_3_VERSION "WaveformSummary-3.0"
#define WAVEFORM_3_DESCRIPTION "Waveform 3.0"
#define WAVEFORMSUMMARY_3_DESCRIPTION "WaveformSummary 3.0"

// Used from Mixxx 1.11 pre
#define WAVEFORM_4_VERSION "Waveform-4.0"
#define WAVEFORMSUMMARY_4_VERSION "WaveformSummary-4.0"
#define WAVEFORM_4_DESCRIPTION "Waveform 4.0"
#define WAVEFORMSUMMARY_4_DESCRIPTION "WaveformSummary 4.0"


class WaveformFactory {
  public:
    enum VersionClass {
        VC_USE,
        VC_KEEP,
        VC_REMOVE
    };

    static bool updateWaveformFromAnalysis(
            Waveform* pWaveform, const AnalysisDao::AnalysisInfo& analysis);
    static VersionClass waveformVersionToVersionClass(const QString& version);
    static VersionClass waveformSummaryVersionToVersionClass(const QString& version);
    static QString currentWaveformVersion();
    static QString currentWaveformDescription();
    static QString currentWaveformSummaryVersion();
    static QString currentWaveformSummaryDescription();
};

#endif /* WAVEFORMFACTORY_H */
