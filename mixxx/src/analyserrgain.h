/*
 * analysergain.h
 *
 *  Created on: 19/apr/2011
 *      Author: vittorio
 */

#ifndef ANALYSERRGAIN_H_
#define ANALYSERRGAIN_H_
#include "analyser.h"
#include "configobject.h"
#include "vamp/vampanalyser.h"

class AnalyserrGain: public Analyser {

public:
    AnalyserrGain(ConfigObject<ConfigValue> *_config);
    ~AnalyserrGain();
    void initialise(TrackPointer tio, int sampleRate, int totalSamples);
    void process(const CSAMPLE *pIn, const int iLen);
    void finalise(TrackPointer tio);

private:
    bool m_bPass;
    ConfigObject<ConfigValue> *m_pConfigAVR;
    VampAnalyser* mvamprg;

    //int m_iStartTime;
};

#endif /* ANALYSERGAIN_H_ */
