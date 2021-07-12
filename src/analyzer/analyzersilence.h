#pragma once

#include "analyzer/analyzer.h"
#include "preferences/usersettings.h"

class CuePointer;

class AnalyzerSilence : public Analyzer {
  public:
    explicit AnalyzerSilence(UserSettingsPointer pConfig);
    ~AnalyzerSilence() override = default;

    bool initialize(TrackPointer pTrack,
            mixxx::audio::SampleRate sampleRate,
            int totalSamples) override;
    bool processSamples(const CSAMPLE* pIn, const int iLen) override;
    void storeResults(TrackPointer pTrack) override;
    void cleanup() override;

  private:
    UserSettingsPointer m_pConfig;
    CSAMPLE m_fThreshold;
    int m_iFramesProcessed;
    bool m_bPrevSilence;
    int m_iSignalStart;
    int m_iSignalEnd;
};
