/* Beat Tracking test via vamp-plugins
 * analyservamptest.h
 *
 *  Created on: 16/mar/2011
 *      Author: Vittorio Colao
 */

#ifndef ANALYSERVAMPTEST_H_
#define ANALYSERVAMPTEST_H_

#include "analyser.h"
#include "configobject.h"
#include "vamp/vampanalyser.h"

class AnalyserVampTest: public Analyser {

public:
    AnalyserVampTest(ConfigObject<ConfigValue> *_config);
    ~AnalyserVampTest();
    void initialise(TrackPointer tio, int sampleRate, int totalSamples);
    void process(const CSAMPLE *pIn, const int iLen);
    void finalise(TrackPointer tio);

private:
    bool m_bPass;
    ConfigObject<ConfigValue> *m_pConfigAVT;
    VampAnalyser* mvamp;
    //int m_iStartTime;
};

#endif /* ANALYSERVAMPTEST_H_ */
