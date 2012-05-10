/*
 * analyserrg.h
 *
 *  Created on: 13/ott/2010
 *      Author: Vittorio Colao
 *       */

#ifndef ANALYSERRG_H_
#define ANALYSERRG_H_

#include "analyser.h"
#include "configobject.h"

class AnalyserGain : public Analyser {
  public:
    AnalyserGain(ConfigObject<ConfigValue> *_config);
    virtual ~AnalyserGain();

    bool initialise(TrackPointer tio, int sampleRate, int totalSamples);
    void process(const CSAMPLE *pIn, const int iLen);
    void cleanup(TrackPointer tio);
    void finalise(TrackPointer tio);

  private:
    int m_iStepControl;
    ConfigObject<ConfigValue> *m_pConfigReplayGain;
    CSAMPLE* m_pLeftTempBuffer;
    CSAMPLE* m_pRightTempBuffer;
    int m_iBufferSize;
};

#endif /* ANALYSERRG_H_ */
