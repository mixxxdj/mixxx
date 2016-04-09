#ifndef ANALYZER_ANALYZERKEY_H
#define ANALYZER_ANALYZERKEY_H

#include <QHash>
#include <QString>

#include "analyzer/analyzer.h"
#include "analyzer/vamp/vampanalyzer.h"
#include "preferences/usersettings.h"
#include "trackinfoobject.h"

class AnalyzerKey : public Analyzer {
  public:
    AnalyzerKey(UserSettingsPointer pConfig);
    virtual ~AnalyzerKey();

    bool initialize(TrackPointer tio, int sampleRate, int totalSamples) override;
    bool loadStored(TrackPointer tio) const override;
    void process(const CSAMPLE *pIn, const int iLen) override;
    void finalize(TrackPointer tio) override;
    void cleanup(TrackPointer tio) override;

  private:
    static QHash<QString, QString> getExtraVersionInfo(
        QString pluginId, bool bPreferencesFastAnalysis);

    UserSettingsPointer m_pConfig;
    VampAnalyzer* m_pVamp;
    QString m_pluginId;
    int m_iSampleRate;
    int m_iTotalSamples;

    bool m_bPreferencesKeyDetectionEnabled;
    bool m_bPreferencesFastAnalysisEnabled;
    bool m_bPreferencesReanalyzeEnabled;
};

#endif /* ANALYZER_ANALYZERKEY_H */
