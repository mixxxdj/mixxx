#include <dsp/onsets/DetectionFunction.h>
#include <dsp/tempotracking/DownBeat.h>
#include <dsp/tempotracking/TempoTrackV2.h>

// Class header comes after library includes here since our preprocessor
// definitions interfere with qm-dsp's headers.
#include "analyzer/plugins/analyzerqueenmarybeats.h"

#include "analyzer/constants.h"

namespace mixxx {
namespace {

// This determines the resolution of the resulting BeatMap.
// ~12 ms (86 Hz) is a fair compromise between accuracy and analysis speed,
// also matching the preferred window/step sizes from BeatTrack VAMP.
// For a 44.1 kHz track, we go in 512 sample steps
// TODO: kStepSecs and the waveform sample rate of 441
// (defined in AnalyzerWaveform::initialize) do not align well and thus
// generate interference. Currently we are at this odd factor: 441 * 0.01161 = 5.12.
// This should be adjusted to be an integer.
constexpr float kStepSecs = 0.01161f;
// results in 43 Hz @ 44.1 kHz / 47 Hz @ 48 kHz / 47 Hz @ 96 kHz
constexpr int kMaximumBinSizeHz = 50; // Hz

DFConfig makeDetectionFunctionConfig(int stepSizeFrames, int windowSize) {
    // These are the defaults for the VAMP beat tracker plugin we used in Mixxx
    // 2.0.
    DFConfig config;
    config.DFType = DF_COMPLEXSD;
    config.stepSize = stepSizeFrames;
    config.frameLength = windowSize;
    config.dbRise = 3;
    config.adaptiveWhitening = false;
    config.whiteningRelaxCoeff = -1;
    config.whiteningFloor = -1;
    return config;
}

constexpr size_t kDecimationFactor = 16;

} // namespace

AnalyzerQueenMaryBeats::AnalyzerQueenMaryBeats()
        : m_windowSize(0),
          m_stepSizeFrames(0) {
}

AnalyzerQueenMaryBeats::~AnalyzerQueenMaryBeats() {
}

bool AnalyzerQueenMaryBeats::initialize(mixxx::audio::SampleRate sampleRate) {
    m_detectionResults.clear();
    m_sampleRate = sampleRate;
    m_stepSizeFrames = static_cast<int>(m_sampleRate * kStepSecs);
    m_windowSize = MathUtilities::nextPowerOfTwo(m_sampleRate / kMaximumBinSizeHz);
    m_pDetectionFunction = std::make_unique<DetectionFunction>(
            makeDetectionFunctionConfig(m_stepSizeFrames, m_windowSize));
    qDebug() << "input sample rate is " << m_sampleRate << ", step size is " << m_stepSizeFrames;

    m_helper.initialize(
            m_windowSize, m_stepSizeFrames, [this](double* pWindow, size_t) {
                // TODO(rryan) reserve?
                m_detectionResults.push_back(
                        m_pDetectionFunction->processTimeDomain(pWindow));
                return true;
            });

    m_pDownBeat = std::make_unique<DownBeat>(
            static_cast<float>(m_sampleRate),
            kDecimationFactor,
            static_cast<size_t>(m_stepSizeFrames));
    m_monoBuffer.clear();
    m_detectedBeatsPerBar = 0;
    m_detectedDownbeatOffset = 0;

    return true;
}

bool AnalyzerQueenMaryBeats::processSamples(const CSAMPLE* pIn, SINT iLen) {
    DEBUG_ASSERT(iLen % kAnalysisChannels == 0);
    if (!m_pDetectionFunction) {
        return false;
    }

    bool result = m_helper.processStereoSamples(pIn, iLen);

    if (m_pDownBeat) {
        const SINT numFrames = iLen / kAnalysisChannels;
        for (SINT i = 0; i < numFrames; ++i) {
            float mono = (pIn[i * kAnalysisChannels] +
                           pIn[i * kAnalysisChannels + 1]) * 0.5f;
            m_monoBuffer.push_back(mono);
            if (static_cast<int>(m_monoBuffer.size()) >= m_stepSizeFrames) {
                m_pDownBeat->pushAudioBlock(m_monoBuffer.data());
                m_monoBuffer.clear();
            }
        }
    }

    return result;
}

bool AnalyzerQueenMaryBeats::finalize() {
    m_helper.finalize();

    std::size_t nonZeroCount = m_detectionResults.size();
    while (nonZeroCount > 0 && m_detectionResults.at(nonZeroCount - 1) <= 0.0) {
        --nonZeroCount;
    }

    std::size_t required_size = std::max(static_cast<std::size_t>(2), nonZeroCount) - 2;

    std::vector<double> df;
    df.reserve(required_size);
    auto beatPeriod = std::vector<int>(required_size / 128 + 1);

    // skip first 2 results as it might have detect noise as onset
    // that's how vamp does and seems works best this way
    for (std::size_t i = 2; i < nonZeroCount; ++i) {
        df.push_back(m_detectionResults.at(i));
    }

    TempoTrackV2 tt(m_sampleRate, m_stepSizeFrames);
    tt.calculateBeatPeriod(df, beatPeriod);

    std::vector<double> beats;
    tt.calculateBeats(df, beatPeriod, beats);

    m_resultBeats.reserve(static_cast<int>(beats.size()));
    for (std::size_t i = 0; i < beats.size(); ++i) {
        // we add the halve m_stepSizeFrames here, because the beat
        // is detected between the two samples.
        const auto result = mixxx::audio::FramePos(
                (beats.at(i) * m_stepSizeFrames) + m_stepSizeFrames / 2);
        m_resultBeats.push_back(result);
    }

    if (m_pDownBeat && beats.size() > 8) {
        size_t audioLength = 0;
        const float* pAudio = m_pDownBeat->getBufferedAudio(audioLength);

        if (pAudio && audioLength > 0) {
            constexpr int kCandidates[] = {3, 4, 5, 6, 7};
            // A confident detection requires the best candidate's downbeat
            // pattern to stand out clearly from the next-best one. If the
            // margin is too small the time signature is ambiguous and we
            // report "unknown" (0) so the global default is used instead of
            // committing to a likely-wrong guess.
            constexpr double kMinConfidenceRatio = 1.15;
            double bestVariance = -1.0;
            double secondBestVariance = -1.0;
            int bestBpb = 0;
            int bestOffset = 0;

            for (int bpb : kCandidates) {
                m_pDownBeat->setBeatsPerBar(bpb);
                std::vector<int> downbeats;
                m_pDownBeat->findDownBeats(
                        pAudio, audioLength, beats, downbeats);

                std::vector<double> beatsd;
                m_pDownBeat->getBeatSD(beatsd);

                if (beatsd.empty()) {
                    continue;
                }

                double sum = 0.0;
                for (double v : beatsd) {
                    sum += v;
                }
                double mean = sum / static_cast<double>(beatsd.size());
                double variance = 0.0;
                for (double v : beatsd) {
                    double diff = v - mean;
                    variance += diff * diff;
                }
                variance /= static_cast<double>(beatsd.size());

                if (variance > bestVariance) {
                    secondBestVariance = bestVariance;
                    bestVariance = variance;
                    bestBpb = bpb;
                    bestOffset = downbeats.empty() ? 0 : downbeats.front();
                } else if (variance > secondBestVariance) {
                    secondBestVariance = variance;
                }
            }

            const bool confident = bestVariance > 0.0 &&
                    (secondBestVariance <= 0.0 ||
                            bestVariance >= secondBestVariance * kMinConfidenceRatio);
            if (confident) {
                m_detectedBeatsPerBar = bestBpb;
                m_detectedDownbeatOffset = bestOffset;
            } else {
                m_detectedBeatsPerBar = 0;
                m_detectedDownbeatOffset = 0;
            }
            qDebug() << "DownBeat analysis:"
                     << (confident ? "detected" : "ambiguous, using default;")
                     << m_detectedBeatsPerBar << "beats per bar,"
                     << "downbeat offset" << m_detectedDownbeatOffset
                     << "(variance:" << bestVariance
                     << "second:" << secondBestVariance << ")";
        }
    }

    m_pDownBeat.reset();
    m_pDetectionFunction.reset();
    return true;
}

} // namespace mixxx
