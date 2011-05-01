/* Key Detection test via vamp-plugins
 * analyservamptest.h
 *
 *  Created on: 15/apr/2011
 *      Author: Vittorio Colao
 */

#ifndef ANALYSERVAMPKEYTEST_H_
#define ANALYSERVAMPKEYTEST_H_

#include "analyser.h"
#include "configobject.h"
#include "vamp/vampanalyser.h"

class AnalyserVampKeyTest: public Analyser {

public:
    AnalyserVampKeyTest(ConfigObject<ConfigValue> *_config);
    ~AnalyserVampKeyTest();
    void initialise(TrackPointer tio, int sampleRate, int totalSamples);
    void process(const CSAMPLE *pIn, const int iLen);
    void finalise(TrackPointer tio);

private:
    bool m_bPass;
    ConfigObject<ConfigValue> *m_pConfigAVT;
    VampAnalyser* mvamp;
    QVector<double> m_frames;
    QVector<double> m_keys;
    //int m_iStartTime;
};

#endif /* ANALYSERVAMPKEYTEST_H_ */
