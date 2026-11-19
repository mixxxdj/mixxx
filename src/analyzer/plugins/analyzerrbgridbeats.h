#pragma once

#include <QObject>
#include <memory>

#include "analyzer/plugins/analyzerplugin.h"

namespace rbgrid {
class Tracker;
} // namespace rbgrid

namespace mixxx {

/// Constant-tempo beat grid matching rekordbox's (see rbgrid::Tracker). The
/// grid starts at rekordbox's beat "1", so bar markers line up.
class AnalyzerRbGridBeats : public AnalyzerBeatsPlugin {
  public:
    static AnalyzerPluginInfo pluginInfo() {
        return AnalyzerPluginInfo(
                "mixxx-rbgrid:1",
                "Mixxx",
                QObject::tr("Constant Grid (rekordbox-compatible)"),
                false);
    }

    AnalyzerRbGridBeats();
    ~AnalyzerRbGridBeats() override;

    AnalyzerPluginInfo info() const override {
        return pluginInfo();
    }

    bool initialize(mixxx::audio::SampleRate sampleRate) override;
    bool processSamples(const CSAMPLE* pIn, SINT iLen) override;
    bool finalize() override;

    bool supportsBeatTracking() const override {
        return false;
    }
    mixxx::Bpm getBpm() const override {
        return m_resultBpm;
    }
    mixxx::audio::FramePos getFirstBeat() const override {
        return m_firstBeat;
    }

  private:
    std::unique_ptr<rbgrid::Tracker> m_pTracker;
    mixxx::audio::SampleRate m_sampleRate;
    mixxx::Bpm m_resultBpm;
    mixxx::audio::FramePos m_firstBeat;
};

} // namespace mixxx
