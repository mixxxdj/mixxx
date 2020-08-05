#pragma once

#include <QHash>
#include <QList>
#include <QMap>
#include <memory>

#include <dsp/tempotracking/DownBeat.h>
#include <dsp/onsets/DetectionFunction.h>
#include <dsp/tempotracking/TempoTrackV2.h>
#include <dsp/transforms/FFT.h>
#include <dsp/chromagram/ConstantQ.h>
#include <base/Window.h>

#include <dsp/tempogram/FIRFilter.h>
#include <dsp/tempogram/WindowFunction.h>
#include <dsp/tempogram/NoveltyCurveProcessor.h>
#include <dsp/tempogram/SpectrogramProcessor.h>
#include <dsp/tempogram/AutocorrelationProcessor.h>

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
    int stepSize();
    int windowSize();

    // beats and bpms methods

    // beats are in detection function increments
    std::vector<double> computeBeats();

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
    std::vector<int> m_downbeats;
    QList<double> m_rawTempos;
    QMap<double, int> m_rawTemposFrenquency;
    QMap<int, double> m_stableTemposAndPositions;

    std::unique_ptr<DetectionFunction> m_pDetectionFunction;
    std::unique_ptr<DownBeat> m_downbeat;
    mixxx::DownmixAndOverlapHelper m_onsetsProcessor;
    mixxx::DownmixAndOverlapHelper m_downbeatsProcessor;
    mixxx::DownmixAndOverlapHelper m_noveltyCurveProcessor;
    std::vector<DFresults> m_detectionResults;
    std::unique_ptr<FFTReal> m_fft;
    std::unique_ptr<ConstantQ> m_constQ;
    Window<double> *m_window;
    double *m_fftRealOut;
    double *m_fftImagOut;
    double *m_constQRealOut;
    double *m_constQImagOut;

    Spectrogram m_spectrogram;
    float m_noveltyCurveMinV;


    
};