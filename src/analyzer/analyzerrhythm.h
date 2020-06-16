#pragma once

#include <dsp/onsets/DetectionFunction.h>
#include <dsp/tempotracking/DownBeat.h>
#include <dsp/tempotracking/TempoTrackV2.h>

#include <QHash>
#include <QList>

#include "analyzer/analyzer.h"
#include "analyzer/plugins/buffering_utils.h"
#include "audio/frame.h"
#include "preferences/usersettings.h"
#include "util/memory.h"
#include "util/samplebuffer.h"

class DetectionFunction;

class AnalyzerRhythm : public Analyzer {
  public:
    explicit AnalyzerRhythm(
            UserSettingsPointer pConfig);
    ~AnalyzerRhythm() override = default;

    bool initialize(const AnalyzerTrack& tio,
            mixxx::audio::SampleRate sampleRate,
            SINT totalSamples) override;
    bool processSamples(const CSAMPLE* pIn, SINT iLen) override;
    void storeResults(TrackPointer pTrack) override;
    void cleanup() override;

  private:
    bool shouldAnalyze(TrackPointer pTrack) const;
    std::vector<double> computeBeats();
    std::vector<double> computeBeatsSpectralDifference(std::vector<double>& beats);
    std::tuple<int, int> computeMeter(std::vector<double>& beatsSD);
    mixxx::audio::SampleRate m_sampleRate;
    int m_iTotalSamples;
    int m_iMaxSamplesToProcess;
    int m_iCurrentSample;
    int m_iMinBpm, m_iMaxBpm;

    std::unique_ptr<DetectionFunction> m_pDetectionFunction;
    std::unique_ptr<DownBeat> m_downbeat;
    mixxx::DownmixAndOverlapHelper m_processor;
    int m_windowSize;
    int m_stepSize;
    std::vector<double> m_detectionResults;
    QVector<mixxx::audio::FramePos> m_resultBeats;
};
