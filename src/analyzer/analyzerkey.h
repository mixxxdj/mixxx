#pragma once

#include <QHash>
#include <QList>
#include <QString>
#include <memory>

#include "analyzer/analyzer.h"
#include "analyzer/plugins/analyzerplugin.h"
#include "preferences/keydetectionsettings.h"
#include "track/track_decl.h"

class AnalyzerKey : public Analyzer {
  public:
    explicit AnalyzerKey(const KeyDetectionSettings& keySettings);
    ~AnalyzerKey() override = default;

    static QList<mixxx::AnalyzerPluginInfo> availablePlugins();
    static mixxx::AnalyzerPluginInfo defaultPlugin();

    bool initialize(const AnalyzerTrack& track,
            mixxx::audio::SampleRate sampleRate,
            SINT frameLength) override;
    bool processSamples(const CSAMPLE* pIn, SINT count) override;
    void storeResults(TrackPointer tio) override;
    void cleanup() override;

  private:
    static QHash<QString, QString> getExtraVersionInfo(
            const QString& pluginId, bool bPreferencesFastAnalysis);

    bool shouldAnalyze(TrackPointer tio) const;

    KeyDetectionSettings m_keySettings;
    std::unique_ptr<mixxx::AnalyzerKeyPlugin> m_pPlugin;
    QString m_pluginId;
    mixxx::audio::SampleRate m_sampleRate;
    SINT m_totalFrames;
    SINT m_maxFramesToProcess;
    SINT m_currentFrame;

    bool m_bPreferencesKeyDetectionEnabled;
    bool m_bPreferencesFastAnalysisEnabled;
    bool m_bPreferencesReanalyzeEnabled;
};
