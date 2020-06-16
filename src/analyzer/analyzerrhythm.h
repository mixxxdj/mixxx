#pragma once

#include <QHash>
#include <QList>

#include <dsp/tempotracking/DownBeat.h>
#include <dsp/onsets/DetectionFunction.h>
#include <dsp/tempotracking/TempoTrackV2.h>

#include "analyzer/analyzer.h"
#include "preferences/usersettings.h"

#include "analyzer/plugins/buffering_utils.h"
#include "util/samplebuffer.h"
#include "util/memory.h"

class DetectionFunction;

class AnalyzerRhythm : public Analyzer {
  public:
    explicit AnalyzerRhythm(
            UserSettingsPointer pConfig);
    ~AnalyzerRhythm() override = default;

    bool initialize(TrackPointer pTrack, int sampleRate, int totalSamples) override;
    bool processSamples(const CSAMPLE *pIn, const int iLen) override;
    void storeResults(TrackPointer pTrack) override;
    void cleanup() override;

  private:
    bool shouldAnalyze(TrackPointer pTrack) const;
    std::vector<double> computeBeats();
    std::vector<double> computeBeatsSpectralDifference(std::vector<double>& beats);
    int m_iSampleRate;
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
    QVector<double> m_resultBeats;
};