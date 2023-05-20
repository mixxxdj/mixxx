#pragma once

#include <QHash>
#include <QList>
#include <QString>

#include "analyzer/analyzer.h"
#include "analyzer/analyzertrack.h"
#include "analyzer/plugins/analyzerplugin.h"
#include "preferences/keydetectionsettings.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"
#include "util/memory.h"

class AnalyzerKey : public Analyzer {
  public:
    explicit AnalyzerKey(const KeyDetectionSettings& keySettings);
    ~AnalyzerKey() override = default;

    static QList<mixxx::AnalyzerPluginInfo> availablePlugins();
    static mixxx::AnalyzerPluginInfo defaultPlugin();

    bool initialize(const AnalyzerTrack& tio,
            mixxx::audio::SampleRate sampleRate,
            SINT totalSamples) override;
    bool processSamples(const CSAMPLE* pIn, SINT iLen) override;
    void storeResults(TrackPointer tio) override;
    void cleanup() override;

  private:
    static QHash<QString, QString> getExtraVersionInfo(
            const QString& pluginId, bool bPreferencesFastAnalysis);

    bool shouldAnalyze(TrackPointer tio) const;

    KeyDetectionSettings m_keySettings;
    std::unique_ptr<mixxx::AnalyzerKeyPlugin> m_pPlugin;
    QString m_pluginId;
    int m_iSampleRate;
    int m_iTotalSamples;
    int m_iMaxSamplesToProcess;
    SINT m_iCurrentSample;

    bool m_bPreferencesKeyDetectionEnabled;
    bool m_bPreferencesFastAnalysisEnabled;
    bool m_bPreferencesReanalyzeEnabled;
};
