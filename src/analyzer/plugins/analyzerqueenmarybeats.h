#ifndef ANALYZER_PLUGINS_ANALYZERQUEENMARYBEATS_H
#define ANALYZER_PLUGINS_ANALYZERQUEENMARYBEATS_H

#include <vector>

#include <QObject>

#include "analyzer/plugins/analyzerplugin.h"
#include "analyzer/plugins/buffering_utils.h"
#include "util/memory.h"
#include "util/samplebuffer.h"

class DetectionFunction;

namespace mixxx {

class AnalyzerQueenMaryBeats : public AnalyzerBeatsPlugin {
  public:
    static AnalyzerPluginInfo pluginInfo() {
        return AnalyzerPluginInfo(
                "qm-tempotracker",
                QObject::tr("Queen Mary University London"),
                QObject::tr("Queen Mary Tempo and Beat Tracker"));
    }

    AnalyzerQueenMaryBeats();
    ~AnalyzerQueenMaryBeats() override;

    AnalyzerPluginInfo info() const override {
        return pluginInfo();
    }

    bool initialize(int samplerate) override;
    bool processSamples(const CSAMPLE* pIn, const int iLen) override;
    bool finalize() override;

    bool supportsBeatTracking() const override {
        return true;
    }

    QVector<double> getBeats() const override {
        return m_resultBeats;
    }

  private:
    std::unique_ptr<DetectionFunction> m_pDetectionFunction;
    DownmixAndOverlapHelper m_helper;
    int m_iSampleRate;
    std::vector<double> m_detectionResults;
    QVector<double> m_resultBeats;
};

} // namespace mixxx

#endif /* ANALYZER_PLUGINS_ANALYZERQUEENMARYBEATS_H */
