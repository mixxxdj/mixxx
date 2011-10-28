/* Beat Tracking test via vamp-plugins
 * analyserbeats.h
 *
 *  Created on: 16/mar/2011
 *      Author: Vittorio Colao
 */

#ifndef ANALYSERBEATS_H_
#define ANALYSERBEATS_H_

#include "analyser.h"
#include "configobject.h"
#include "vamp/vampanalyser.h"

class AnalyserBeats: public Analyser {

public:
    AnalyserBeats(ConfigObject<ConfigValue> *_config);
    ~AnalyserBeats();
    void initialise(TrackPointer tio, int sampleRate, int totalSamples);
    void process(const CSAMPLE *pIn, const int iLen);
    void finalise(TrackPointer tio);

private:
    bool m_bPass;
    ConfigObject<ConfigValue> *m_pConfigAVT;
    VampAnalyser* mvamp;
    //int m_iStartTime;
    int m_iSampleRate;

    double calculateBpm(QVector<double> beats) const;
};

#endif /* ANALYSERVAMPTEST_H_ */
