/* Beat Tracking test via vamp-plugins
 * analyserbeats.h
 *
 *  Created on: 16/mar/2011
 *      Author: Vittorio Colao
 */

#ifndef ANALYSERKEY_H_
#define ANALYSERKEY_H_

#include "analyser.h"
#include "configobject.h"
#include "vamp/vampanalyser.h"
//#include "../vamp-plugins/plugins/keyfinder.h"
//#include "../vamp-plugins/plugins/samplerate.h"

class AnalyserKey: public Analyser {
  public:
    AnalyserKey(ConfigObject<ConfigValue> *_config);
    virtual ~AnalyserKey();

    bool initialise(TrackPointer tio, int sampleRate, int totalSamples);
    void process(const CSAMPLE *pIn, const int iLen);
    void finalise(TrackPointer tio);
    void cleanup(TrackPointer tio);	

  private:
    //QVector<double> correctedBeats ( QVector<double> rawbeats, bool bypass);
    bool m_bPass;
    //ConfigObject<ConfigValue> *m_pConfigAVT;
    VampAnalyser* m_pVamp;
  /*  KeyFinder::KeyFinder keyfinderKey;
    KeyFinder::Parameters keyfinderParams;
    KeyFinder::AudioData keyfinderAudio;
    KeyFinder::KeyDetectionResult keyfinderResult ;
    int outSamples ;
    int totSamples ;*/

    QVector<double> m_frames;
    QVector<double> m_keys;
    bool m_bPreferencesKeyDetectionEnabled, m_bPreferencesfastAnalysisEnabled, m_bPreferencesfirstLastEnabled, m_bPreferencesreanalyzeEnabled, m_bPreferencesskipRelevantEnabled;
    ConfigObject<ConfigValue> *m_pConfig;
    bool m_bShouldAnalyze;

    /*SRC_DATA src_in;
    SRC_STATE *src_state ;
    int converter, channels, error;
    double downsamplingRatio;*/


};

#endif /* ANALYSERVAMPTEST_H_ */
