/*
 * analysergain.h
 *
 *  Created on: 19/apr/2011
 *      Author: vittorio
 */

#ifndef ANALYSERRGAINVAMP_H_
#define ANALYSERRGAINVAMP_H_

#include "analyser.h"
#include "configobject.h"
#include "vamp/vampanalyser.h"

class AnalyserGainVamp: public Analyser {
  public:
    AnalyserGainVamp(ConfigObject<ConfigValue> *_config);
    ~AnalyserGainVamp();
    bool initialise(TrackPointer tio, int sampleRate, int totalSamples);
    void process(const CSAMPLE *pIn, const int iLen);
    void finalise(TrackPointer tio);

  private:
    bool m_bPass;
    ConfigObject<ConfigValue> *m_pConfigAVR;
    VampAnalyser* mvamprg;
};

#endif /* ANALYSERRGAINVAMP_H_ */
