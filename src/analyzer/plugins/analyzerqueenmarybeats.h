#ifndef ANALYZER_PLUGINS_ANALYZERQUEENMARYBEATS_H
#define ANALYZER_PLUGINS_ANALYZERQUEENMARYBEATS_H

#include <vector>

#include <QObject>
#include <QScopedPointer>

#include "analyzer/plugins/analyzerplugin.h"
#include "analyzer/plugins/buffering_utils.h"
#include "util/samplebuffer.h"

class DetectionFunction;

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
    bool process(const CSAMPLE* pIn, const int iLen) override;
    bool finalize() override;

    bool supportsBeatTracking() const override {
        return true;
    }

    QVector<double> getBeats() const override {
        return m_resultBeats;
    }

  private:
    QScopedPointer<DetectionFunction> m_pDetectionFunction;
    DownmixAndOverlapHelper<double, CSAMPLE> m_helper;
    size_t m_stepSize = 0;
    int m_iSampleRate = 0;
    std::vector<double> m_detectionResults;
    QVector<double> m_resultBeats;
};

#endif /* ANALYZER_PLUGINS_ANALYZERQUEENMARYBEATS_H */
