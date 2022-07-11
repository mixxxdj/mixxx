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
#include "analyzer/plugins/analyzerpluginsupport.h"
#include "preferences/beatdetectionsettings.h"
#include "preferences/usersettings.h"
#include "util/memory.h"

class AnalyzerBeats : private AnalyzerPluginSupportInfo, public Analyzer {
  public:
    explicit AnalyzerBeats(
            UserSettingsPointer pConfig,
            bool enforceBpmDetection = false);
    ~AnalyzerBeats() override = default;

    bool initialize(TrackPointer pTrack,
            mixxx::audio::SampleRate sampleRate,
            int totalSamples) override;
    bool processSamples(const CSAMPLE *pIn, const int iLen) override;
    void storeResults(TrackPointer tio) override;
    void cleanup() override;
    static QList<mixxx::AnalyzerPluginInfo> defaultPluginsList();

  private:
    QList<mixxx::AnalyzerPluginInfo> availablePlugins() const override;
    bool shouldAnalyze(TrackPointer pTrack) const;

    BeatDetectionSettings m_bpmSettings;
    std::unique_ptr<mixxx::AnalyzerBeatsPlugin> m_pPlugin;
    const bool m_enforceBpmDetection;
    bool m_bPreferencesReanalyzeOldBpm;
    bool m_bPreferencesReanalyzeImported;
    bool m_bPreferencesFixedTempo;
    bool m_bPreferencesFastAnalysis;

    mixxx::audio::SampleRate m_sampleRate;
    SINT m_totalSamples;
    int m_iMaxSamplesToProcess;
    int m_iCurrentSample;
};
