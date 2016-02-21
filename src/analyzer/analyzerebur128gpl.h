#ifndef ANALYZEREBUR128GPL_H_
#define ANALYZEREBUR128GPL_H_

#include "analyzer/analyzer.h"
#include "preferences/usersettings.h"

class Ebu_r128_proc;

class AnalyzerEbur128Gpl : public Analyzer {
  public:
    AnalyzerEbur128Gpl(UserSettingsPointer config);
    virtual ~AnalyzerEbur128Gpl();

    bool initialize(TrackPointer tio, int sampleRate, int totalSamples) override;
    bool loadStored(TrackPointer tio) const override;
    void process(const CSAMPLE* pIn, const int iLen) override;
    void cleanup(TrackPointer tio) override;
    void finalize(TrackPointer tio) override;

  private:
    UserSettingsPointer m_pConfig;
    bool m_initalized;
    CSAMPLE* m_pTempBuffer[2];
    Ebu_r128_proc*  m_pEbu128Proc;
    int m_iBufferSize;
};

#endif /* ANALYZEREBUR128GPL_H_ */
