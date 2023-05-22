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
            SINT frameLength) override;
    bool processSamples(const CSAMPLE* pIn, SINT count) override;
    void storeResults(TrackPointer pTrack) override;
    void cleanup() override;

    static void setupMainAndIntroCue(Track* pTrack, double firstSound, UserSettings* pConfig);
    static void setupOutroCue(Track* pTrack, double lastSound);

  private:
    UserSettingsPointer m_pConfig;
    CSAMPLE m_fThreshold;
    SINT m_framesProcessed;
    bool m_bPrevSilence;
    SINT m_signalStart;
    SINT m_signalEnd;
};
