#ifndef ANALYZER_ANALYZERSILENCE_H
#define ANALYZER_ANALYZERSILENCE_H

#include "analyzer/analyzer.h"
#include "preferences/usersettings.h"

class CuePointer;

class AnalyzerSilence : public Analyzer {
  public:
    AnalyzerSilence(UserSettingsPointer pConfig);
    virtual ~AnalyzerSilence() override;

    bool initialize(TrackPointer tio, int sampleRate, int totalSamples) override;
    bool isDisabledOrLoadStoredSuccess(TrackPointer tio) const override;
    void process(const CSAMPLE* pIn, const int iLen) override;
    void finalize(TrackPointer tio) override;
    void cleanup(TrackPointer tio) override;

  private:
    UserSettingsPointer m_pConfig;
    float m_fThreshold;
    int m_iFramesProcessed;
    bool m_bPrevSilence;
    int m_iSignalStart;
    int m_iSignalEnd;

    TrackPointer m_pTrack;
    CuePointer m_pIntroCue;
    CuePointer m_pOutroCue;
};

#endif // ANALYZER_ANALYZERSILENCE_H
