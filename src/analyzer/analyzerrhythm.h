#pragma once

#include <QHash>
#include <QList>
#include <QMap>
#include <memory>

#include <dsp/tempotracking/DownBeat.h>
#include <dsp/onsets/DetectionFunction.h>
#include <dsp/tempotracking/TempoTrackV2.h>

#include "analyzer/analyzer.h"
#include "preferences/usersettings.h"

#include "analyzer/plugins/buffering_utils.h"
#include "util/samplebuffer.h"

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
    // general methods
    bool shouldAnalyze(TrackPointer pTrack) const;

    // beats and bpms methods

    // beats are in detection function increments
    std::vector<double> computeBeats();
    std::vector<double> computeSnapGrid();

    // these methods are defined at analyzerrhythmbpm
    // TODO(Cristiano) Hide these!! Maybe a friend class?
    // smoothed beats positions are in frame increments
    std::tuple<QVector<double>, QMap<int, double>> FixBeatsPositions();
    std::tuple<QList<double>, QMap<double, int>> computeRawTemposAndFrequency(
            const QVector<double>& beats, const int beatWindow = 2);
    double tempoMedian(const QVector<double>& beats, const int beatWindow = 2);
    double tempoMode(const QVector<double>& beats, const int beatWindow = 2);
    QMap<int, double> findTempoChanges();
    QVector<double> calculateFixedTempoGrid(const QVector<double>& rawbeats, 
            const double localBpm, bool correctFirst = true);
    double findFirstCorrectBeat(
            const QVector<double> rawbeats, const double global_bpm);
    double calculateBpm(const QVector<double>& beats, bool tryToRound = true);
    std::tuple<double, QMap<double, int>> computeFilteredWeightedAverage(
            const QMap<double, int>& tempoFrequency, const double filterCenter);

    // downbeats and meter methods
    std::vector<double> computeBeatsSpectralDifference(std::vector<double>& beats);
    std::tuple<int, int> computeMeter(std::vector<double>& beatsSD);
    
    int m_iSampleRate;
    int m_iTotalSamples;
    int m_iMaxSamplesToProcess;
    int m_iCurrentSample;
    int m_iMinBpm, m_iMaxBpm;
    int m_beatsPerBar;
    
    QVector<double> m_resultBeats;
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
