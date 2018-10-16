#include <dsp/onsets/DetectionFunction.h>
#include <dsp/tempotracking/TempoTrackV2.h>

// Class header comes after library includes here since our preprocessor
// definitions interfere with qm-dsp's headers.
#include "analyzer/plugins/analyzerqueenmarybeats.h"

namespace mixxx {

AnalyzerQueenMaryBeats::AnalyzerQueenMaryBeats() {
}

AnalyzerQueenMaryBeats::~AnalyzerQueenMaryBeats() {
}

bool AnalyzerQueenMaryBeats::initialize(int samplerate) {
    qDebug() << "AnalyzerQueenMaryBeats::initialize" << samplerate;
    m_detectionResults.clear();
    m_iSampleRate = samplerate;

    // These are the preferred window/step sizes from the BeatTrack VAMP plugin.
    size_t windowSize = 1024;
    m_stepSize = 512;

    // These are the defaults for the VAMP beat tracker plugin we used in Mixxx
    // 2.0.
    DFConfig config;
    config.DFType = DF_SPECDIFF;
    config.stepSize = m_stepSize;
    config.frameLength = windowSize;
    config.dbRise = 3;
    config.adaptiveWhitening = 0;
    config.whiteningRelaxCoeff = -1;
    config.whiteningFloor = -1;
    m_pDetectionFunction = std::make_unique<DetectionFunction>(config);

    m_helper.initialize(
        windowSize, m_stepSize, [this](double* pWindow, size_t) {
            // TODO(rryan) reserve?
            m_detectionResults.push_back(
                m_pDetectionFunction->processTimeDomain(pWindow));
            return true;
        });
    return true;
}

bool AnalyzerQueenMaryBeats::process(const CSAMPLE* pIn, const int iLen) {
    DEBUG_ASSERT(iLen % 2 == 0);
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

    qDebug() << "AnalyzerQueenMaryBeats::finalize nonZeroCount" << nonZeroCount;

    std::vector<double> df;
    std::vector<double> beatPeriod;
    std::vector<double> tempi;

    // NOTE(rryan): The VAMP plugin skipped the first 2 detection function
    // results so I do as well. Not sure why.
    for (int i = 2; i < nonZeroCount; ++i) {
        df.push_back(m_detectionResults.at(i));
        beatPeriod.push_back(0.0);
    }

    if (df.empty()) {
        return false;
    }

    TempoTrackV2 tt(m_iSampleRate, m_stepSize);

    tt.calculateBeatPeriod(df, beatPeriod, tempi);

    std::vector<double> beats;
    tt.calculateBeats(df, beatPeriod, beats);

    qDebug() << "AnalyzerQueenMaryBeats::finalize beats" << beats.size();

    // TODO(rryan) better copy
    m_resultBeats.resize(beats.size());
    double* result = (double*)&m_resultBeats.at(0);
    for (size_t i = 0; i < beats.size(); ++i) {
        result[i] = beats[i] * m_stepSize;

        if (i % 10 == 0) {
            qDebug() << "AQMB::finalize beat" << i << beats[i] << result[i];
        }
    }

    qDebug() << "AnalyzerQueenMaryBeats::finalize resultBeats" << m_resultBeats.size();
    m_pDetectionFunction.reset();
    return true;
}

}  // namespace mixxx
