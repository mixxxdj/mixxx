#ifndef ANALYZER_ANALYZEREBUR128_H_
#define ANALYZER_ANALYZEREBUR128_H_

#include <ebur128.h>

#include "analyzer/analyzer.h"
#include "preferences/replaygainsettings.h"

class AnalyzerEbur128 : public Analyzer {
  public:
    AnalyzerEbur128(UserSettingsPointer pConfig);
    virtual ~AnalyzerEbur128();

    static bool isEnabled(const ReplayGainSettings& rgSettings) {
        return rgSettings.isAnalyzerEnabled(2);
    }

    bool initialize(TrackPointer tio, int sampleRate, int totalSamples) override;
    bool processSamples(const CSAMPLE* pIn, const int iLen) override;
    void storeResults(TrackPointer tio) override;
    void cleanup() override;

  private:
    ReplayGainSettings m_rgSettings;
    ebur128_state* m_pState;
};

#endif /* ANALYZER_ANALYZEREBUR128_H_ */
