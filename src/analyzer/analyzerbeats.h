/* Beat Tracking test via vamp-plugins
 * analyzerbeats.h
 *
 *  Created on: 16/mar/2011
 *      Author: Vittorio Colao
 */

#pragma once

#include <QHash>
#include <QList>

#include "analyzer/analyzer.h"
#include "analyzer/plugins/analyzerplugin.h"
#include "preferences/beatdetectionsettings.h"
#include "preferences/usersettings.h"
#include "util/memory.h"

class AnalyzerBeats : public Analyzer {
  public:
    explicit AnalyzerBeats(
            UserSettingsPointer pConfig,
            bool enforceBpmDetection = false);
    ~AnalyzerBeats() override = default;

    static QList<mixxx::AnalyzerPluginInfo> availablePlugins();
    static mixxx::AnalyzerPluginInfo defaultPlugin();

    bool initialize(TrackPointer pTrack, int sampleRate, int totalSamples) override;
    bool processSamples(const CSAMPLE *pIn, const int iLen) override;
    void storeResults(TrackPointer tio) override;
    void cleanup() override;

  private:
    bool shouldAnalyze(TrackPointer pTrack) const;
    static QHash<QString, QString> getExtraVersionInfo(
            const QString& pluginId, bool bPreferencesFastAnalysis);

    BeatDetectionSettings m_bpmSettings;
    std::unique_ptr<mixxx::AnalyzerBeatsPlugin> m_pPlugin;
    const bool m_enforceBpmDetection;
    QString m_pluginId;
    bool m_bPreferencesReanalyzeOldBpm;
    bool m_bPreferencesReanalyzeImported;
    bool m_bPreferencesFixedTempo;
    bool m_bPreferencesFastAnalysis;

    mixxx::audio::SampleRate m_sampleRate;
    SINT m_totalSamples;
    int m_iMaxSamplesToProcess;
    int m_iCurrentSample;
};
