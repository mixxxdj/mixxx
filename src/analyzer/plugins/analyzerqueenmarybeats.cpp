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
    DEBUG_ASSERT(iLen == kAnalysisSamplesPerBlock);
    DEBUG_ASSERT(iLen % kAnalysisChannels == 0);
    if (!m_pDetectionFunction) {
        return false;
    }

    return m_helper.processStereoSamples(pIn, iLen);
}

bool AnalyzerQueenMaryBeats::finalize() {
    // TODO(rryan) if iLen is less than frame size, pad with zeros. Do we need
    // flush support?

    int nonZeroCount = m_detectionResults.size();
    while (nonZeroCount > 0 && m_detectionResults.at(nonZeroCount - 1) <= 0.0) {
        --nonZeroCount;
    }

    std::vector<double> df;
    std::vector<double> beatPeriod;
    std::vector<double> tempi;

    df.reserve(nonZeroCount);
    beatPeriod.reserve(nonZeroCount);

    // NOTE(rryan): The VAMP plugin skipped the first 2 detection function
    // results so I do as well. Not sure why.
    for (int i = 2; i < nonZeroCount; ++i) {
        df.push_back(m_detectionResults.at(i));
        beatPeriod.push_back(0.0);
    }

    if (df.empty()) {
        return false;
    }

    TempoTrackV2 tt(m_iSampleRate, kStepSize);
    tt.calculateBeatPeriod(df, beatPeriod, tempi);

    std::vector<double> beats;
    tt.calculateBeats(df, beatPeriod, beats);

    m_resultBeats.resize(beats.size());
    double* result = (double*)&m_resultBeats.at(0);
    for (size_t i = 0; i < beats.size(); ++i) {
        result[i] = beats[i] * kStepSize;
    }

    m_pDetectionFunction.reset();
    return true;
}

} // namespace mixxx
