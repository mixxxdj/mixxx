#ifndef ANALYSEREBUR128_H_
#define ANALYSEREBUR128_H_

#include "analyser.h"
#include "configobject.h"

class Ebu_r128_proc;

class AnalyserEbur128 : public Analyser {
  public:
    AnalyserEbur128(ConfigObject<ConfigValue> *config);
    virtual ~AnalyserEbur128();

    virtual bool initialise(TrackPointer tio, int sampleRate, int totalSamples);
    virtual void process(const CSAMPLE *pIn, const int iLen);
    virtual bool loadStored(TrackPointer tio) const;
    virtual void cleanup(TrackPointer tio);
    virtual void finalise(TrackPointer tio);

  private:
    ConfigObject<ConfigValue>* m_pConfig;
    bool m_initalized;
    CSAMPLE* m_pTempBuffer[2];
    Ebu_r128_proc*  m_pEbu128Proc;
    int m_iBufferSize;
};

#endif /* ANALYSEREBUR128_H_ */
