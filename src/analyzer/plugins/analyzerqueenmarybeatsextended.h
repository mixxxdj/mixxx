#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <memory>
#include <vector>

#include "analyzer/plugins/analyzerplugin.h"
#include "analyzer/plugins/buffering_utils.h"

class DetectionFunction;

namespace mixxx {

struct AnalysisBPMSegment {
    double startTime;
    double endTime;
    double duration;
    double startBPM;
    double endBPM;
    QString type; // "STABLE", "INCREASE", "DECREASE"

    AnalysisBPMSegment()
            : startTime(0),
              endTime(0),
              duration(0),
              startBPM(0),
              endBPM(0) {
    }
};

struct MusicalContext {
    int timeSignature;    // 3 or 4
    int beatsPerBar;      // 3 or 4
    bool halfTimeFeel;    // true for dubstep style
    double perceivedBpm;  // For musical phrasing
    int beatsPerPhrase;   // 24 (3/4), 32 (4/4), 16 (half-time)
    double phraseSeconds; // Duration of one musical phrase
};

class AnalyzerQueenMaryBeatsExtended : public AnalyzerBeatsPlugin {
  public:
    static AnalyzerPluginInfo pluginInfo() {
        return AnalyzerPluginInfo(
                "qm-tempotracker-extended:0",
                QObject::tr("Queen Mary University London (Extended)"),
                QObject::tr("Queen Mary Tempo and Beat Tracker with BPM Segmentation"),
                true);
    }

    AnalyzerQueenMaryBeatsExtended();
    ~AnalyzerQueenMaryBeatsExtended() override;

    AnalyzerPluginInfo info() const override {
        return pluginInfo();
    }

    bool initialize(mixxx::audio::SampleRate sampleRate) override;
    bool processSamples(const CSAMPLE* pIn, SINT iLen) override;
    bool finalize() override;

    bool supportsBeatTracking() const override {
        return true;
    }

    QVector<mixxx::audio::FramePos> getBeats() const override {
        return m_resultBeats;
    }

    QVector<AnalysisBPMSegment> getBpmSegments() const {
        return m_bpmSegments;
    }

    QJsonArray getBpmSegmentsJson() const;

  private:
    void analyzeBpmChanges();
    void smoothBpmData(std::vector<double>& bpmValues, int windowLength);
    void detectMusicalContext();
    double calculateLocalBpm(const std::vector<double>& beatTimes, int startIdx, int windowSize);
    double calculateConfidence(const std::vector<double>& intervals);
    double findNearestBeat(double time, const std::vector<double>& beatTimes);
    void snapSegmentsToBeats();
    double getBpmAtTime(double time,
            const std::vector<double>& bpmTimes,
            const std::vector<double>& bpmValues);

    std::unique_ptr<DetectionFunction> m_pDetectionFunction;
    DownmixAndOverlapHelper m_helper;
    mixxx::audio::SampleRate m_sampleRate;
    int m_windowSize;
    int m_stepSizeFrames;
    std::vector<double> m_detectionResults;
    QVector<mixxx::audio::FramePos> m_resultBeats;
    std::vector<double> m_beatTimes;

    QVector<AnalysisBPMSegment> m_bpmSegments;
    double m_trackDuration;
    MusicalContext m_musicalContext;

    // Analysis parameters
    static constexpr int kWindowSizeBeats = 12;
    static constexpr int kStepSizeBeats = 3;
    static constexpr int kSmoothWindowLength = 5;
    static constexpr double kMinRateChange = 0.5;
    static constexpr double kStablePercentPerMinute = 0.9;
    static constexpr double kMaxSnapDistance = 0.3;
};

} // namespace mixxx
