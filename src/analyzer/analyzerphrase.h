#pragma once

#include <QVector>

#include "analyzer/analyzer.h"
#include "preferences/usersettings.h"

class AnalyzerPhrase : public Analyzer {
  public:
    explicit AnalyzerPhrase(UserSettingsPointer pConfig);
    ~AnalyzerPhrase() override = default;

    static bool isEnabled(const UserSettingsPointer& pConfig);

    bool initialize(const AnalyzerTrack& track,
            mixxx::audio::SampleRate sampleRate,
            mixxx::audio::ChannelCount channelCount,
            SINT frameLength) override;
    bool processSamples(const CSAMPLE* pIn, SINT count) override;
    void storeResults(TrackPointer pTrack) override;
    void cleanup() override;

  private:
    UserSettingsPointer m_pConfig;
    mixxx::audio::SampleRate m_sampleRate;
    mixxx::audio::ChannelCount m_channelCount;
    SINT m_totalFrames;

    static constexpr SINT kEnergyWindowFrames = 2048;
    QVector<float> m_energyWindows;
    double m_currentWindowSum;
    SINT m_framesInCurrentWindow;
};
