#ifndef ANALYZER_PLUGINS_ANALYZERSOUNDTOUCHBEATS
#define ANALYZER_PLUGINS_ANALYZERSOUNDTOUCHBEATS

#include <QObject>
#include <QScopedPointer>

#include "analyzer/plugins/analyzerplugin.h"
#include "util/samplebuffer.h"

namespace soundtouch {
class BPMDetect;
}  // namespace soundtouch

class AnalyzerSoundTouchBeats : public AnalyzerBeatsPlugin {
  public:
    static AnalyzerPluginInfo pluginInfo() {
        return AnalyzerPluginInfo(
            "mixxxbpmdetection",
            "Olli Parviainen",
            QObject::tr("SoundTouch BPM Detector (Legacy)"));
    }

    AnalyzerSoundTouchBeats();
    ~AnalyzerSoundTouchBeats() override;

    AnalyzerPluginInfo info() const override {
        return pluginInfo();
    }

    bool initialize(int samplerate) override;
    bool process(const CSAMPLE* pIn, const int iLen) override;
    bool finalize() override;

    bool supportsBeatTracking() const override {
        return false;
    }

    float getBpm() const override {
        return m_fResultBpm;
    }

  private:
    QScopedPointer<soundtouch::BPMDetect> m_pSoundTouch;
    SampleBuffer m_downmixBuffer;
    float m_fResultBpm = 0.0f;
};

#endif /* ANALYZER_PLUGINS_ANALYZERSOUNDTOUCHBEATS */
