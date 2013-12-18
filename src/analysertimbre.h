#ifndef ANALYSERTIMBRE_H
#define ANALYSERTIMBRE_H

#include "analyser.h"
#include "configobject.h"
#include "vamp/vampanalyser.h"

class AnalyserTimbre : public Analyser {
  public:
    AnalyserTimbre(ConfigObject<ConfigValue> *pConfig);
    ~AnalyserTimbre();

    bool initialise(TrackPointer tio, int sampleRate, int totalSamples);
    bool loadStored(TrackPointer tio) const;
    void process(const CSAMPLE *pIn, const int iLen);
    void cleanup(TrackPointer tio);
    void finalise(TrackPointer tio);

  private:
    static QHash<QString, QString> getExtraVersionInfo(
        QString pluginId, bool bPreferencesFastAnalysis);
    bool checkStoredTimbre(TrackPointer tio) const;

    ConfigObject<ConfigValue> *m_pConfig;
    VampAnalyser* m_pVamp;
    QString m_pluginId;
    int m_iSampleRate;
    int m_iTotalSamples;
    bool m_bShouldAnalyze;
    bool m_bPreferencesFastAnalysis;
};

#endif // ANALYSERTIMBRE_H
