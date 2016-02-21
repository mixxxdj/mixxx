#ifndef ANALYZEREBUR128MIT_H_
#define ANALYZEREBUR128MIT_H_

#include "analyzer/analyzer.h"
#include "preferences/usersettings.h"
#include <ebur128.h>

class AnalyzerEbur128Mit : public Analyzer {
  public:
    AnalyzerEbur128Mit(UserSettingsPointer config);
    virtual ~AnalyzerEbur128Mit();

    bool initialize(TrackPointer tio, int sampleRate, int totalSamples) override;
    bool loadStored(TrackPointer tio) const override;
    void process(const CSAMPLE* pIn, const int iLen) override;
    void cleanup(TrackPointer tio) override;
    void finalize(TrackPointer tio) override;

  private:
    UserSettingsPointer m_pConfig;
    bool m_initalized;
    CSAMPLE* m_pTempBuffer[2];
    int m_iBufferSize;
    ebur128_state* m_pState;
};

#endif /* ANALYZEREBUR128MIT_H_ */
