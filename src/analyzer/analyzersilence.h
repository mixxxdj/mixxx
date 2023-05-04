#pragma once

#include "analyzer/analyzer.h"
#include "preferences/usersettings.h"

class CuePointer;

class AnalyzerSilence : public Analyzer {
  public:
    explicit AnalyzerSilence(UserSettingsPointer pConfig);
    ~AnalyzerSilence() override = default;

    bool initialize(TrackPointer pTrack, int sampleRate, int totalSamples) override;
    bool processSamples(const CSAMPLE* pIn, const int iLen) override;
    void storeResults(TrackPointer pTrack) override;
    void cleanup() override;

    static void setupMainAndIntroCue(Track* pTrack, double firstSound, UserSettings* pConfig);
    static void setupOutroCue(Track* pTrack, double lastSound);

  private:
    UserSettingsPointer m_pConfig;
    CSAMPLE m_fThreshold;
    int m_iFramesProcessed;
    bool m_bPrevSilence;
    int m_iSignalStart;
    int m_iSignalEnd;
};
