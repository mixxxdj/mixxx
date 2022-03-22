#pragma once

#include <QObject>

#include "analyzer/plugins/analyzerplugin.h"
#include "util/memory.h"
#include "util/samplebuffer.h"

namespace soundtouch {
class BPMDetect;
} // namespace soundtouch

namespace mixxx {

class AnalyzerSoundTouchBeats : public AnalyzerBeatsPlugin {
  public:
    static AnalyzerPluginInfo pluginInfo() {
        return AnalyzerPluginInfo(
                "mixxxbpmdetection",
                "Olli Parviainen",
                QObject::tr("SoundTouch BPM Detector (Legacy)"),
                false);
    }

    AnalyzerSoundTouchBeats();
    ~AnalyzerSoundTouchBeats() override;

    AnalyzerPluginInfo info() const override {
        return pluginInfo();
    }

    bool initialize(mixxx::audio::SampleRate sampleRate) override;
    bool processSamples(const CSAMPLE* pIn, const int iLen) override;
    bool finalize() override;

    bool supportsBeatTracking() const override {
        return false;
    }

    mixxx::Bpm getBpm() const override {
        return m_resultBpm;
    }

  private:
    std::unique_ptr<soundtouch::BPMDetect> m_pSoundTouch;
    /// mono, i.e. 1 sample per frame
    SampleBuffer m_downmixBuffer;
    mixxx::Bpm m_resultBpm;
};

} // namespace mixxx
