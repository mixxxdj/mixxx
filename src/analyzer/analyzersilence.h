#ifndef ANALYZER_ANALYZERSILENCE_H
#define ANALYZER_ANALYZERSILENCE_H

#include "analyzer/analyzer.h"
#include "preferences/usersettings.h"

class CuePointer;

class AnalyzerSilence : public Analyzer {
  public:
    explicit AnalyzerSilence(UserSettingsPointer pConfig);
    ~AnalyzerSilence() override = default;

    bool initialize(TrackPointer pTrack, int sampleRate, int totalSamples) override;
    bool isDisabledOrLoadStoredSuccess(TrackPointer pTrack) const override;
    void process(const CSAMPLE* pIn, const int iLen) override;
    void finalize(TrackPointer pTrack) override;
    void cleanup(TrackPointer pTrack) override;

  private:
    static bool shouldUpdateCue(CuePosition cuePosipTrackn);

    UserSettingsPointer m_pConfig;
    float m_fThreshold;
    int m_iFramesProcessed;
    bool m_bPrevSilence;
    int m_iSignalStart;
    int m_iSignalEnd;
};

#endif // ANALYZER_ANALYZERSILENCE_H
