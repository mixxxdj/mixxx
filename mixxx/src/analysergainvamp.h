/*
 * analysergain.h
 *
 *  Created on: 19/apr/2011
 *      Author: vittorio
 */

#ifndef ANALYSERRGAINVAMP_H
#define ANALYSERRGAINVAMP_H

#include "analyser.h"
#include "configobject.h"
#include "vamp/vampanalyser.h"

class AnalyserGainVamp: public Analyser {
  public:
    AnalyserGainVamp(ConfigObject<ConfigValue> *_config);
    virtual ~AnalyserGainVamp();

    bool initialise(TrackPointer tio, int sampleRate, int totalSamples);
    void process(const CSAMPLE *pIn, const int iLen);
    void finalise(TrackPointer tio);

  private:
    ConfigObject<ConfigValue>* m_pConfig;
    bool m_bShouldAnalyze;
    VampAnalyser* m_pVamp;
};

#endif /* ANALYSERRGAINVAMP_H */
