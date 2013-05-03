#ifndef ANALYSERKEY_H
#define ANALYSERKEY_H

#include "analyser.h"
#include "configobject.h"
#include "trackinfoobject.h"
#include "vamp/vampanalyser.h"

class AnalyserKey : public Analyser {
  public:
    AnalyserKey(ConfigObject<ConfigValue> *_config);
    virtual ~AnalyserKey();

    bool initialise(TrackPointer tio, int sampleRate, int totalSamples);
    bool loadStored(TrackPointer tio) const;
    void process(const CSAMPLE *pIn, const int iLen);
    void finalise(TrackPointer tio);
    void cleanup(TrackPointer tio);

  private:
    ConfigObject<ConfigValue> *m_pConfig;
    bool m_bPass;
    VampAnalyser* m_pVamp;
    QString m_pluginId;
    int m_iSampleRate;
    int m_iTotalSamples;
    QVector<double> m_frames;
    QVector<double> m_keys;

    bool m_bPreferencesKeyDetectionEnabled;
    bool m_bPreferencesFastAnalysisEnabled;
    bool m_bPreferencesfirstLastEnabled;
    bool m_bPreferencesreanalyzeEnabled;
    bool m_bPreferencesskipRelevantEnabled;
    bool m_bShouldAnalyze;
};

#endif /* ANALYSERKEY_H */
