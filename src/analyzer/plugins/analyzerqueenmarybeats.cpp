#include <dsp/onsets/DetectionFunction.h>
#include <dsp/tempotracking/TempoTrackV2.h>

// Class header comes after library includes here since our preprocessor
// definitions interfere with qm-dsp's headers.
#include "analyzer/plugins/analyzerqueenmarybeats.h"

#include "analyzer/constants.h"

namespace mixxx {
namespace {
// These are the preferred window/step sizes from the BeatTrack VAMP plugin.
constexpr size_t kWindowSize = 1024;
constexpr size_t kStepSize = 512;

DFConfig makeDetectionFunctionConfig() {
    // These are the defaults for the VAMP beat tracker plugin we used in Mixxx
    // 2.0.
    DFConfig config;
    config.DFType = DF_SPECDIFF;
    config.stepSize = kStepSize;
    config.frameLength = kWindowSize;
    config.dbRise = 3;
    config.adaptiveWhitening = 0;
    config.whiteningRelaxCoeff = -1;
    config.whiteningFloor = -1;
    return config;
}

} // namespace

AnalyzerQueenMaryBeats::AnalyzerQueenMaryBeats()
        : m_iSampleRate(0) {
}

AnalyzerQueenMaryBeats::~AnalyzerQueenMaryBeats() {
}

bool AnalyzerQueenMaryBeats::initialize(int samplerate) {
    m_detectionResults.clear();
    m_iSampleRate = samplerate;
    m_pDetectionFunction = std::make_unique<DetectionFunction>(
            makeDetectionFunctionConfig());

    m_helper.initialize(
            kWindowSize, kStepSize, [this](double* pWindow, size_t) {
                // TODO(rryan) reserve?
                m_detectionResults.push_back(
                        m_pDetectionFunction->processTimeDomain(pWindow));
                return true;
            });
    return true;
}

bool AnalyzerQueenMaryBeats::processSamples(const CSAMPLE* pIn, const int iLen) {
    DEBUG_ASSERT(iLen % kAnalysisChannels == 0);
    if (!m_pDetectionFunction) {
        return false;
    }

    return m_helper.processStereoSamples(pIn, iLen);
}

bool AnalyzerQueenMaryBeats::finalize() {
    m_helper.finalize();

    int nonZeroCount = m_detectionResults.size();
    while (nonZeroCount > 0 && m_detectionResults.at(nonZeroCount - 1) <= 0.0) {
        --nonZeroCount;
    }

    std::vector<double> df;
    std::vector<double> beatPeriod(nonZeroCount);
    std::vector<double> tempi;

    df.reserve(nonZeroCount);

    for (int i = 0; i < nonZeroCount; ++i) {
        df.push_back(m_detectionResults.at(i));
    }

    TempoTrackV2 tt(m_iSampleRate, kStepSize);
    tt.calculateBeatPeriod(df, beatPeriod, tempi);

    std::vector<double> beats;
    tt.calculateBeats(df, beatPeriod, beats);

    // In some tracks a beat at 0:00 is detected when a noise floor starts.
    // Here we check the level and the position for plausibility and remove
    // the beat if this is the case.
    size_t firstBeat = 0;
    if (beats.size() >= 3) {
        if (beats.at(0) <= 0) {
            firstBeat = 1;
        } else if (m_detectionResults.at(beats.at(0)) <
                (m_detectionResults.at(beats.at(1)) +
                m_detectionResults.at(beats.at(2))) / 4) {
            // the beat is not half es high than the average of the two
            // following beats. Skip it.
            firstBeat = 1;
        } else {
            int diff = (beats.at(1) - beats.at(0)) - (beats.at(2) - beats.at(1));
            // we don't allow a significant tempo change after the first beat
            if (diff > 2 || diff < -2) {
                // first beat is off grid. Skip it.
                firstBeat = 1;
            }
        }
    }

    m_resultBeats.reserve(beats.size());
    for (size_t i = firstBeat; i < beats.size(); ++i) {
        double result = (beats.at(i) * kStepSize) - kStepSize / 2;
        m_resultBeats.push_back(result);
    }

    m_pDetectionFunction.reset();
    return true;
}

} // namespace mixxx
