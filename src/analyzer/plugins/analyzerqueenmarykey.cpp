#include <dsp/keydetection/GetKeyMode.h>

// Class header comes after library includes here since our preprocessor
// definitions interfere with qm-dsp's headers.
#include "analyzer/plugins/analyzerqueenmarykey.h"

#include "analyzer/constants.h"
#include "util/assert.h"

using mixxx::track::io::key::ChromaticKey;
using mixxx::track::io::key::ChromaticKey_IsValid;

namespace mixxx {
namespace {

// Tuning frequency of concert A in Hertz. Default value from VAMP plugin.
constexpr int kTuningFrequencyHertz440 = 440;
// Alternative tuning frequency for 432Hz music
constexpr int kTuningFrequencyHertz432 = 432;

} // namespace

AnalyzerQueenMaryKey::AnalyzerQueenMaryKey()
        : m_currentFrame(0),
          m_prevKey(mixxx::track::io::key::INVALID),
          m_currentFrame432(0),
          m_prevKey432(mixxx::track::io::key::INVALID),
          m_bDetect432Hz(false),
          m_bIs432Hz(false) {
}

AnalyzerQueenMaryKey::~AnalyzerQueenMaryKey() {
}

bool AnalyzerQueenMaryKey::initialize(mixxx::audio::SampleRate sampleRate) {
    m_prevKey = mixxx::track::io::key::INVALID;
    m_resultKeys.clear();
    m_currentFrame = 0;
    m_bIs432Hz = false;

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

    // Initialize 440Hz analyzer
    GetKeyMode::Config config440(sampleRate, kTuningFrequencyHertz440);
    m_pKeyMode = std::make_unique<GetKeyMode>(config440);
    size_t windowSize = m_pKeyMode->getBlockSize();
    size_t stepSize = m_pKeyMode->getHopSize();

    bool success = m_helper.initialize(
            windowSize, stepSize, [this](double* pWindow, size_t) {
                int iKey = m_pKeyMode->process(pWindow);

                if (!ChromaticKey_IsValid(iKey)) {
                    qWarning() << "No valid key detected in analyzed window:" << iKey;
                    DEBUG_ASSERT(!"iKey is invalid");
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

    if (!success) {
        return false;
    }

    // Initialize 432Hz analyzer if detection is enabled
    if (m_bDetect432Hz) {
        m_prevKey432 = mixxx::track::io::key::INVALID;
        m_resultKeys432.clear();
        m_currentFrame432 = 0;

        GetKeyMode::Config config432(sampleRate, kTuningFrequencyHertz432);
        m_pKeyMode432 = std::make_unique<GetKeyMode>(config432);
        size_t windowSize432 = m_pKeyMode432->getBlockSize();
        size_t stepSize432 = m_pKeyMode432->getHopSize();

        success = m_helper432.initialize(
                windowSize432, stepSize432, [this](double* pWindow, size_t) {
                    int iKey = m_pKeyMode432->process(pWindow);

                    if (!ChromaticKey_IsValid(iKey)) {
                        return false;
                    }
                    const auto key = static_cast<ChromaticKey>(iKey);
                    if (key != m_prevKey432) {
                        m_resultKeys432.push_back(qMakePair(
                                key, static_cast<double>(m_currentFrame432)));
                        m_prevKey432 = key;
                    }
                    return true;
                });
    }

    return success;
}

bool AnalyzerQueenMaryKey::processSamples(const CSAMPLE* pIn, SINT iLen) {
    DEBUG_ASSERT(iLen % kAnalysisChannels == 0);
    if (!m_pKeyMode) {
        return false;
    }

    const size_t numInputFrames = iLen / kAnalysisChannels;
    m_currentFrame += numInputFrames;
    bool success = m_helper.processStereoSamples(pIn, iLen);

    // Also process with 432Hz analyzer if enabled
    if (m_bDetect432Hz && m_pKeyMode432) {
        m_currentFrame432 += numInputFrames;
        m_helper432.processStereoSamples(pIn, iLen);
    }

    return success;
}

bool AnalyzerQueenMaryKey::finalize() {
    m_helper.finalize();
    m_pKeyMode.reset();

    // Finalize 432Hz analyzer if enabled and compare results
    if (m_bDetect432Hz && m_pKeyMode432) {
        m_helper432.finalize();
        m_pKeyMode432.reset();

        // Compare results: The analysis with fewer key changes is likely more accurate
        // because a correct tuning reference results in more consistent key detection.
        // If both have the same number of changes, prefer 440Hz (the standard).
        if (!m_resultKeys432.isEmpty() && !m_resultKeys.isEmpty()) {
            // Calculate consistency score: fewer key changes = better detection
            // Also consider that a valid key (not INVALID) at the start is better
            bool has440ValidStart = !m_resultKeys.isEmpty() &&
                    m_resultKeys.first().first != mixxx::track::io::key::INVALID;
            bool has432ValidStart = !m_resultKeys432.isEmpty() &&
                    m_resultKeys432.first().first != mixxx::track::io::key::INVALID;

            int keyChanges440 = m_resultKeys.size();
            int keyChanges432 = m_resultKeys432.size();

            // 432Hz is detected if:
            // 1. The 432Hz analysis has fewer key changes (more consistent), OR
            // 2. Both have same changes but 432Hz has a valid start and 440Hz doesn't
            // We require 432Hz to be significantly better (at least 20% fewer changes)
            // to avoid false positives
            bool is432HzBetter = false;
            if (has432ValidStart && !has440ValidStart) {
                is432HzBetter = true;
            } else if (keyChanges432 < keyChanges440) {
                // 432Hz needs to be at least 20% better to be considered
                double ratio = static_cast<double>(keyChanges432) / static_cast<double>(keyChanges440);
                is432HzBetter = (ratio < 0.8);
            }

            if (is432HzBetter) {
                m_bIs432Hz = true;
                // Use the 432Hz results
                m_resultKeys = m_resultKeys432;
            }
        }
        m_resultKeys432.clear();
    }

    return true;
}

} // namespace mixxx
