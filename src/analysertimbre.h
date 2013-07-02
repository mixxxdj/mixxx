#ifndef ANALYSERTIMBRE_H
#define ANALYSERTIMBRE_H

#include "analyser.h"
#include "configobject.h"

#include "vamp/vampanalyser.h"

class AnalyserTimbre : public Analyser
{
public:
    AnalyserTimbre(ConfigObject<ConfigValue> *pConfig);
    ~AnalyserBeats();
    bool initialise(TrackPointer tio, int sampleRate, int totalSamples);
    bool loadStored(TrackPointer tio) const;
    void process(const CSAMPLE *pIn, const int iLen);
    void cleanup(TrackPointer tio);
    void finalise(TrackPointer tio);
private:
    ConfigObject<ConfigValue> *m_pConfig;
    VampAnalyser* m_pVamp;
    QString m_pluginId;
    bool m_bShouldAnalyze;
    bool m_bPreferencesFastAnalysis;
    int m_iSampleRate, m_iTotalSamples;
};

#endif // ANALYSERTIMBRE_H
