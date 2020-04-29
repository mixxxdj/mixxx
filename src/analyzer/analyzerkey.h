#ifndef ANALYZER_ANALYZERKEY_H
#define ANALYZER_ANALYZERKEY_H

#include <QHash>
#include <QList>
#include <QString>

#include "analyzer/analyzer.h"
#include "analyzer/plugins/analyzerplugin.h"
#include "preferences/keydetectionsettings.h"
#include "preferences/usersettings.h"
#include "track/track.h"
#include "util/memory.h"

class AnalyzerKey : public Analyzer {
  public:
    explicit AnalyzerKey(KeyDetectionSettings keySettings);
    ~AnalyzerKey() override = default;

    static QList<mixxx::AnalyzerPluginInfo> availablePlugins();

    bool initialize(TrackPointer tio, int sampleRate, int totalSamples) override;
    bool processSamples(const CSAMPLE *pIn, const int iLen) override;
    void storeResults(TrackPointer tio) override;
    void cleanup() override;

  private:
    static QHash<QString, QString> getExtraVersionInfo(
            QString pluginId, bool bPreferencesFastAnalysis);

    bool shouldAnalyze(TrackPointer tio) const;

    KeyDetectionSettings m_keySettings;
    std::unique_ptr<mixxx::AnalyzerKeyPlugin> m_pPlugin;
    QString m_pluginId;
    int m_iSampleRate;
    int m_iTotalSamples;
    int m_iMaxSamplesToProcess;
    int m_iCurrentSample;

    bool m_bPreferencesKeyDetectionEnabled;
    bool m_bPreferencesFastAnalysisEnabled;
    bool m_bPreferencesReanalyzeEnabled;
};

#endif /* ANALYZER_ANALYZERKEY_H */
