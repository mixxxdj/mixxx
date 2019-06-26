#ifndef ANALYZER_ANALYZERSILENCE_H
#define ANALYZER_ANALYZERSILENCE_H

#include "analyzer/analyzer.h"
#include "preferences/usersettings.h"

class CuePointer;

class AnalyzerSilence : public Analyzer {
  public:
    explicit AnalyzerSilence(UserSettingsPointer pConfig);
    ~AnalyzerSilence() override = default;

    bool initialize(TrackPointer tio, int sampleRate, int totalSamples) override;
    bool processSamples(const CSAMPLE* pIn, const int iLen) override;
    void storeResults(TrackPointer tio) override;
    void cleanup() override;

  private:
    UserSettingsPointer m_pConfig;
    float m_fThreshold;
    int m_iFramesProcessed;
    bool m_bPrevSilence;
    int m_iSignalStart;
    int m_iSignalEnd;
};

#endif // ANALYZER_ANALYZERSILENCE_H
