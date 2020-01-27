#ifndef ANALYZER_PLUGINS_ANALYZERSOUNDTOUCHBEATS
#define ANALYZER_PLUGINS_ANALYZERSOUNDTOUCHBEATS

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

    bool initialize(int samplerate) override;
    bool processSamples(const CSAMPLE* pIn, const int iLen) override;
    bool finalize() override;

    bool supportsBeatTracking() const override {
        return false;
    }

    float getBpm() const override {
        return m_fResultBpm;
    }

  private:
    std::unique_ptr<soundtouch::BPMDetect> m_pSoundTouch;
    SampleBuffer m_downmixBuffer;
    float m_fResultBpm;
};

} // namespace mixxx

#endif /* ANALYZER_PLUGINS_ANALYZERSOUNDTOUCHBEATS */
