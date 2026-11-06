#pragma once

#include <QString>
#include <vector>

#include "analyzer/analyzer.h"
#include "preferences/usersettings.h"

namespace mixxx::hotcues {

constexpr int kMaxCues = 8;

struct AutoCue {
    int beat;      // index into per-beat arrays
    QString label; // start, drop, break or phrase
};

/// first downbeat with sound, then biggest level changes on downbeats, 8+ bars apart
std::vector<AutoCue> placeCues(const std::vector<double>& loudDb,
        const std::vector<double>& bassDb);

} // namespace mixxx::hotcues

/// hot cues at section changes on tracks without any, off with [Library] AutoHotcues 0
class AnalyzerHotcues : public Analyzer {
  public:
    explicit AnalyzerHotcues(UserSettingsPointer pConfig);
    ~AnalyzerHotcues() override = default;

    bool initialize(const AnalyzerTrack& track,
            mixxx::audio::SampleRate sampleRate,
            mixxx::audio::ChannelCount channelCount,
            SINT frameLength) override;
    bool processSamples(const CSAMPLE* pIn, SINT count) override;
    void storeResults(TrackPointer pTrack) override;
    void cleanup() override {
    }

  private:
    UserSettingsPointer m_pConfig;
    int m_channels;
    double m_lowpass;
    double m_low1;
    double m_low2;
    double m_sum;
    double m_bassSum;
    int m_blockFrames;
    std::vector<float> m_power; // mean square per kBlock
    std::vector<float> m_bassPower;
};
