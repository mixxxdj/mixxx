#pragma once

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

// Used from Mixxx 1.12 alpha
#define WAVEFORM_5_VERSION "Waveform-5.0"
#define WAVEFORMSUMMARY_5_VERSION "WaveformSummary-5.0"
#define WAVEFORM_5_DESCRIPTION "Waveform 5.0"
#define WAVEFORMSUMMARY_5_DESCRIPTION "WaveformSummary 5.0"

#define WAVEFORM_CURRENT_VERSION WAVEFORM_5_VERSION
#define WAVEFORMSUMMARY_CURRENT_VERSION WAVEFORMSUMMARY_5_VERSION
#define WAVEFORM_CURRENT_DESCRIPTION WAVEFORM_5_DESCRIPTION
#define WAVEFORMSUMMARY_CURRENT_DESCRIPTION WAVEFORMSUMMARY_5_DESCRIPTION


class WaveformFactory {
  public:
    enum VersionClass {
        VC_USE,
        VC_KEEP,
        VC_REMOVE
    };

    static Waveform* loadWaveformFromAnalysis(
            const AnalysisDao::AnalysisInfo& analysis);
    static VersionClass waveformVersionToVersionClass(const QString& version);
    static VersionClass waveformSummaryVersionToVersionClass(const QString& version);
    static QString currentWaveformVersion();
    static QString currentWaveformDescription();
    static QString currentWaveformSummaryVersion();
    static QString currentWaveformSummaryDescription();
};
