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
    virtual ~AnalyserBeats();
    bool initialise(TrackPointer tio, int sampleRate, int totalSamples);
    bool loadStored(TrackPointer tio) const;
    void process(const CSAMPLE *pIn, const int iLen);
    void cleanup(TrackPointer tio);
    void finalise(TrackPointer tio);

  private:
    QVector<double> correctedBeats(QVector<double> rawbeats);

    ConfigObject<ConfigValue> *m_pConfig;
    VampAnalyser* m_pVamp;
    QString m_pluginId;
    bool m_bShouldAnalyze;
    bool m_bPreferencesReanalyzeOldBpm;
    bool m_bPreferencesFixedTempo;
    bool m_bPreferencesOffsetCorrection;
    bool m_bPreferencesFastAnalysis;

    int m_iSampleRate, m_iTotalSamples;
    int m_iMinBpm, m_iMaxBpm;
};

#endif /* ANALYSERVAMPTEST_H_ */
