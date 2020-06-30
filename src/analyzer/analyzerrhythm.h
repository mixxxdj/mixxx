#pragma once

#include <dsp/onsets/DetectionFunction.h>
#include <dsp/tempotracking/DownBeat.h>
#include <dsp/tempotracking/TempoTrackV2.h>

#include <QHash>
#include <QList>
#include <QMap>
#include <memory>

#include "analyzer/analyzer.h"
#include "analyzer/plugins/buffering_utils.h"
#include "audio/frame.h"
#include "preferences/usersettings.h"
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
    // general methods
    bool shouldAnalyze(TrackPointer pTrack) const;

    // beats and bpms methods

    // beats are in detection function increments
    std::vector<double> computeBeats();

    // these methods are defined at analyzerrhythmbpm
    // TODO(Cristiano) Hide these!! Maybe a friend class?
    // smoothed beats positions are in frame increments
    std::tuple<QVector<mixxx::audio::FramePos>, QMap<int, double>> FixBeatsPositions();
    std::tuple<QList<double>, QMap<double, int>> computeRawTemposAndFrequency(
            const QVector<mixxx::audio::FramePos>& beats, const int beatWindow = 2);
    double tempoMedian(const QVector<mixxx::audio::FramePos>& beats, const int beatWindow = 2);
    double tempoMode(const QVector<mixxx::audio::FramePos>& beats, const int beatWindow = 2);
    QMap<int, double> findTempoChanges();
    QVector<mixxx::audio::FramePos> calculateFixedTempoGrid(
            const QVector<mixxx::audio::FramePos>& rawbeats,
            const double localBpm,
            bool correctFirst = true);
    mixxx::audio::FramePos findFirstCorrectBeat(
            const QVector<mixxx::audio::FramePos> rawbeats, const double global_bpm);
    double calculateBpm(const QVector<mixxx::audio::FramePos>& beats, bool tryToRound = true);
    std::tuple<double, QMap<double, int>> computeFilteredWeightedAverage(
            const QMap<double, int>& tempoFrequency, const double filterCenter);

    // downbeats and meter methods
    std::vector<double> computeBeatsSpectralDifference(std::vector<double>& beats);
    std::tuple<int, int> computeMeter(std::vector<double>& beatsSD);
    mixxx::audio::SampleRate m_sampleRate;
    int m_iTotalSamples;
    int m_iMaxSamplesToProcess;
    int m_iCurrentSample;
    int m_iMinBpm, m_iMaxBpm;
    int m_beatsPerBar;

    QVector<mixxx::audio::FramePos> m_resultBeats;
    QVector<double> m_downbeats;
    QList<double> m_rawTempos;
    QMap<double, int> m_rawTemposFrenquency;
    QMap<int, double> m_stableTemposAndPositions;

    std::unique_ptr<DetectionFunction> m_pDetectionFunction;
    std::unique_ptr<DownBeat> m_downbeat;
    mixxx::DownmixAndOverlapHelper m_processor;
    int m_windowSize;
    int m_stepSize;
    std::vector<DFresults> m_detectionResults;
};
