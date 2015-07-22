#ifndef ANALYSERKEY_H
#define ANALYSERKEY_H

#include <QHash>
#include <QString>

#include "analyser.h"
#include "configobject.h"
#include "trackinfoobject.h"
#include "vamp/vampanalyser.h"

class AnalyserKey : public Analyser {
  public:
    AnalyserKey(ConfigObject<ConfigValue>* pConfig);
    virtual ~AnalyserKey();

    bool initialise(TrackPointer tio, int sampleRate, int totalSamples);
    bool loadStored(TrackPointer tio) const;
    void process(const CSAMPLE *pIn, const int iLen);
    void finalise(TrackPointer tio);
    void cleanup(TrackPointer tio);

  private:
    static QHash<QString, QString> getExtraVersionInfo(
        QString pluginId, bool bPreferencesFastAnalysis);

    ConfigObject<ConfigValue>* m_pConfig;
    VampAnalyser* m_pVamp;
    QString m_pluginId;
    int m_iSampleRate;
    int m_iTotalSamples;

    bool m_bPreferencesKeyDetectionEnabled;
    bool m_bPreferencesFastAnalysisEnabled;
    bool m_bPreferencesReanalyzeEnabled;
};

#endif /* ANALYSERKEY_H */
