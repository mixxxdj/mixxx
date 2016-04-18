#ifndef ANALYZER_ANALYZEREBUR128_H_
#define ANALYZER_ANALYZEREBUR128_H_

#include <ebur128.h>

#include "analyzer/analyzer.h"
#include "preferences/replaygainsettings.h"

class AnalyzerEbur128 : public Analyzer {
  public:
    AnalyzerEbur128(UserSettingsPointer pConfig);
    virtual ~AnalyzerEbur128();

    bool initialize(TrackPointer tio, int sampleRate, int totalSamples) override;
    bool isDisabledOrLoadStoredSuccess(TrackPointer tio) const override;
    void process(const CSAMPLE* pIn, const int iLen) override;
    void cleanup(TrackPointer tio) override;
    void finalize(TrackPointer tio) override;

  private:
    ReplayGainSettings m_rgSettings;
    bool m_initalized;
    int m_iBufferSize;
    ebur128_state* m_pState;
};

#endif /* ANALYZER_ANALYZEREBUR128_H_ */
