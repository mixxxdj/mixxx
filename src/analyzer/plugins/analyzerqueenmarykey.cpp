#include <dsp/keydetection/GetKeyMode.h>

// Class header comes after library includes here since our preprocessor
// definitions interfere with qm-dsp's headers.
#include "analyzer/plugins/analyzerqueenmarykey.h"

#include "analyzer/constants.h"
#include "util/assert.h"
#include "util/math.h"

using mixxx::track::io::key::ChromaticKey;
using mixxx::track::io::key::ChromaticKey_IsValid;

namespace mixxx {
namespace {

// Tuning frequency of concert A in Hertz. Default value from VAMP plugin.
constexpr int kTuningFrequencyHertz = 440;

} // namespace

AnalyzerQueenMaryKey::AnalyzerQueenMaryKey()
        : m_currentFrame(0),
          m_prevKey(mixxx::track::io::key::INVALID) {
}

AnalyzerQueenMaryKey::~AnalyzerQueenMaryKey() {
}

bool AnalyzerQueenMaryKey::initialize(mixxx::audio::SampleRate sampleRate) {
    m_prevKey = mixxx::track::io::key::INVALID;
    m_resultKeys.clear();
    m_currentFrame = 0;

    struct Config {
        double sampleRate;
        float tuningFrequency;
        double hpcpAverage;
        double medianAverage;
        int frameOverlapFactor; // 1 = none (default, fast, but means
                                // we skip a fair bit of input data);
                                // 8 = normal chroma overlap
        int decimationFactor;

        Config(mixxx::audio::SampleRate _sampleRate, float _tuningFrequency)
                : sampleRate(_sampleRate.toDouble()),
                  tuningFrequency(_tuningFrequency),
                  hpcpAverage(10),
                  medianAverage(10),
                  frameOverlapFactor(1),
                  decimationFactor(8) {
        }
    };

    GetKeyMode::Config config(sampleRate, kTuningFrequencyHertz);
    m_pKeyMode = std::make_unique<GetKeyMode>(config);
    size_t windowSize = m_pKeyMode->getBlockSize();
    size_t stepSize = m_pKeyMode->getHopSize();

    return m_helper.initialize(
            windowSize, stepSize, [this](double* pWindow, size_t) {
                int iKey = m_pKeyMode->process(pWindow);

                VERIFY_OR_DEBUG_ASSERT(ChromaticKey_IsValid(iKey)) {
                    qWarning() << "No valid key detected in analyzed window:" << iKey;
                    return false;
                }
                const auto key = static_cast<ChromaticKey>(iKey);
                if (key != m_prevKey) {
                    // TODO(rryan) reserve?
                    m_resultKeys.push_back(qMakePair(
                            key, static_cast<double>(m_currentFrame)));
                    m_prevKey = key;
                }
                return true;
            });
}

bool AnalyzerQueenMaryKey::processSamples(const CSAMPLE* pIn, const int iLen) {
    DEBUG_ASSERT(iLen % kAnalysisChannels == 0);
    if (!m_pKeyMode) {
        return false;
    }

    const size_t numInputFrames = iLen / kAnalysisChannels;
    m_currentFrame += numInputFrames;
    return m_helper.processStereoSamples(pIn, iLen);
}

bool AnalyzerQueenMaryKey::finalize() {
    m_helper.finalize();
    m_pKeyMode.reset();
    return true;
}

} // namespace mixxx
