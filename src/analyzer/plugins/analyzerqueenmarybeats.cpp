#include <dsp/onsets/DetectionFunction.h>
#include <dsp/tempotracking/TempoTrackV2.h>

// Class header comes after library includes here since our preprocessor
// definitions interfere with qm-dsp's headers.
#include "analyzer/plugins/analyzerqueenmarybeats.h"

#include "analyzer/constants.h"

namespace mixxx {
namespace {
// stepsize should be equivalent to ~10ms and this constant (11.61ms) make the number 
// of samples an exact power of 2 for 22.1k, 44.1k, and very close to, for 96k and 192k
constexpr float kStepSecs = 0.01161; 

DFConfig makeDetectionFunctionConfig(int stepSize, int windowSize) {
    // These are the defaults for the VAMP beat tracker plugin we used in Mixxx
    // 2.0.
    DFConfig config;
    config.DFType = DF_COMPLEXSD;
    config.stepSize = stepSize;
    config.frameLength = windowSize;
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
    // These are the preferred window/step sizes from the BeatTrack VAMP
    m_stepSize = int(m_iSampleRate * kStepSecs + 0.0001);
    m_windowSize = m_stepSize * 2;
    m_pDetectionFunction = std::make_unique<DetectionFunction>(
            makeDetectionFunctionConfig(m_stepSize, m_windowSize));
    qDebug() << "input sample rate is " << m_iSampleRate << ", step size is " << m_stepSize;

    m_helper.initialize(
            m_windowSize, m_stepSize, [this](double* pWindow, size_t) {
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
    std::vector<double> beatPeriod;
    std::vector<double> tempi;
    const auto required_size = std::max(0, nonZeroCount - 2);
    df.reserve(required_size);
    beatPeriod.reserve(required_size);

    // skip first 2 results as it might have detect noise as onset
    // that's how vamp does and seems works best this way
    for (int i = 2; i < nonZeroCount; ++i) {
        df.push_back(m_detectionResults.at(i));
        beatPeriod.push_back(0.0);
    }

    TempoTrackV2 tt(m_iSampleRate, m_stepSize);
    tt.calculateBeatPeriod(df, beatPeriod, tempi);

    std::vector<double> beats;
    tt.calculateBeats(df, beatPeriod, beats);

    m_resultBeats.reserve(beats.size());
    for (size_t i = 0; i < beats.size(); ++i) {
        double result = (beats.at(i) * m_stepSize) - m_stepSize / 2;
        m_resultBeats.push_back(result);
    }

    m_pDetectionFunction.reset();
    return true;
}

} // namespace mixxx
