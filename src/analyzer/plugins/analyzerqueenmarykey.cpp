#include <dsp/keydetection/GetKeyMode.h>

#include <algorithm>

// Class header comes after library includes here since our preprocessor
// definitions interfere with qm-dsp's headers.
#include "analyzer/plugins/analyzerqueenmarykey.h"

#include "analyzer/constants.h"
#include "util/assert.h"

using mixxx::track::io::key::ChromaticKey;
using mixxx::track::io::key::ChromaticKey_IsValid;

namespace mixxx {
namespace {

// Standard tuning frequency (concert pitch A4)
constexpr int kStandardTuningFrequency = 440;

// Default detection range
constexpr int kDefaultMinFrequency = 427;  // ~-52 cents from 440Hz
constexpr int kDefaultMaxFrequency = 447;  // ~+28 cents from 440Hz
constexpr int kDefaultStepFrequency = 1;

} // namespace

// FrequencyAnalyzer special member function definitions
// These must be in the cpp file where GetKeyMode is a complete type
AnalyzerQueenMaryKey::FrequencyAnalyzer::FrequencyAnalyzer()
        : currentFrame(0),
          prevKey(mixxx::track::io::key::INVALID),
          frequencyHz(440) {
}

AnalyzerQueenMaryKey::FrequencyAnalyzer::~FrequencyAnalyzer() = default;

AnalyzerQueenMaryKey::FrequencyAnalyzer::FrequencyAnalyzer(FrequencyAnalyzer&& other) noexcept = default;

AnalyzerQueenMaryKey::FrequencyAnalyzer&
AnalyzerQueenMaryKey::FrequencyAnalyzer::operator=(FrequencyAnalyzer&& other) noexcept = default;

AnalyzerQueenMaryKey::AnalyzerQueenMaryKey()
        : m_bDetect432Hz(false),
          m_bDetectTuningFrequency(false),
          m_tuningMinFreq(kDefaultMinFrequency),
          m_tuningMaxFreq(kDefaultMaxFrequency),
          m_tuningStepFreq(kDefaultStepFrequency),
          m_detectedTuningFrequency(kStandardTuningFrequency) {
}

AnalyzerQueenMaryKey::~AnalyzerQueenMaryKey() {
}

bool AnalyzerQueenMaryKey::initialize(mixxx::audio::SampleRate sampleRate) {
    m_sampleRate = sampleRate;
    m_resultKeys.clear();
    m_frequencyAnalyzers.clear();
    m_detectedTuningFrequency = kStandardTuningFrequency;

    // GetKeyMode configuration structure
    struct Config {
        double sampleRate;
        float tuningFrequency;
        double hpcpAverage;
        double medianAverage;
        int frameOverlapFactor;
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

    // Build list of frequencies to analyze
    std::vector<int> frequenciesToTest;

    if (m_bDetectTuningFrequency) {
        // Dynamic tuning detection: test range of frequencies
        for (int freq = m_tuningMinFreq; freq <= m_tuningMaxFreq; freq += m_tuningStepFreq) {
            frequenciesToTest.push_back(freq);
        }
        // Ensure 440Hz and 432Hz are always included
        auto contains = [&](int f) {
            return std::find(frequenciesToTest.begin(), frequenciesToTest.end(), f) !=
                    frequenciesToTest.end();
        };
        if (!contains(440)) {
            frequenciesToTest.push_back(440);
        }
        if (!contains(432)) {
            frequenciesToTest.push_back(432);
        }
    } else if (m_bDetect432Hz) {
        // Legacy mode: only test 440Hz and 432Hz
        frequenciesToTest.push_back(440);
        frequenciesToTest.push_back(432);
    } else {
        // Default: only test 440Hz
        frequenciesToTest.push_back(440);
    }

    // Initialize an analyzer for each frequency
    for (int freq : frequenciesToTest) {
        FrequencyAnalyzer analyzer;
        analyzer.frequencyHz = freq;
        analyzer.currentFrame = 0;
        analyzer.prevKey = mixxx::track::io::key::INVALID;
        analyzer.resultKeys.clear();

        GetKeyMode::Config config(sampleRate, static_cast<float>(freq));
        analyzer.keyMode = std::make_unique<GetKeyMode>(config);

        size_t windowSize = analyzer.keyMode->getBlockSize();
        size_t stepSize = analyzer.keyMode->getHopSize();

        // Capture analyzer index for the lambda
        size_t analyzerIndex = m_frequencyAnalyzers.size();
        m_frequencyAnalyzers.push_back(std::move(analyzer));

        // Initialize the helper with a callback
        bool success = m_frequencyAnalyzers[analyzerIndex].helper.initialize(
                windowSize,
                stepSize,
                [this, analyzerIndex](double* pWindow, size_t) {
                    FrequencyAnalyzer& fa = m_frequencyAnalyzers[analyzerIndex];
                    int iKey = fa.keyMode->process(pWindow);

                    if (!ChromaticKey_IsValid(iKey)) {
                        return false;
                    }
                    const auto key = static_cast<ChromaticKey>(iKey);
                    if (key != fa.prevKey) {
                        fa.resultKeys.push_back(qMakePair(
                                key, static_cast<double>(fa.currentFrame)));
                        fa.prevKey = key;
                    }
                    return true;
                });

        if (!success) {
            return false;
        }
    }

    return !m_frequencyAnalyzers.empty();
}

bool AnalyzerQueenMaryKey::processSamples(const CSAMPLE* pIn, SINT iLen) {
    DEBUG_ASSERT(iLen % kAnalysisChannels == 0);

    if (m_frequencyAnalyzers.empty()) {
        return false;
    }

    const size_t numInputFrames = iLen / kAnalysisChannels;

    // Process samples through all frequency analyzers
    for (size_t i = 0; i < m_frequencyAnalyzers.size(); ++i) {
        m_frequencyAnalyzers[i].currentFrame += numInputFrames;
        m_frequencyAnalyzers[i].helper.processStereoSamples(pIn, iLen);
    }

    return true;
}

bool AnalyzerQueenMaryKey::finalize() {
    // Finalize all analyzers
    for (size_t i = 0; i < m_frequencyAnalyzers.size(); ++i) {
        m_frequencyAnalyzers[i].helper.finalize();
        m_frequencyAnalyzers[i].keyMode.reset();
    }

    // Find the best tuning frequency
    m_detectedTuningFrequency = findBestTuningFrequency();

    // Use the results from the best frequency analyzer
    for (const FrequencyAnalyzer& fa : m_frequencyAnalyzers) {
        if (fa.frequencyHz == m_detectedTuningFrequency) {
            m_resultKeys = fa.resultKeys;
            break;
        }
    }

    // Clean up
    m_frequencyAnalyzers.clear();

    return true;
}

int AnalyzerQueenMaryKey::findBestTuningFrequency() const {
    if (m_frequencyAnalyzers.empty()) {
        return kStandardTuningFrequency;
    }

    // If only one analyzer, return its frequency
    if (m_frequencyAnalyzers.size() == 1) {
        return m_frequencyAnalyzers[0].frequencyHz;
    }

    // Score each frequency based on analysis consistency
    // Lower key change count = more consistent detection = better match
    int bestFrequency = kStandardTuningFrequency;
    int bestScore = INT_MAX;
    bool bestHasValidStart = false;

    for (const FrequencyAnalyzer& fa : m_frequencyAnalyzers) {
        int keyChangeCount = static_cast<int>(fa.resultKeys.size());
        bool hasValidStart = !fa.resultKeys.empty() &&
                fa.resultKeys.front().first != mixxx::track::io::key::INVALID;

        // Calculate score (lower is better)
        // Weight: fewer key changes is strongly preferred
        // Bonus for having a valid key at the start
        int score = keyChangeCount * 100;
        if (!hasValidStart) {
            score += 50;  // Penalty for no valid start
        }

        // Prefer standard tuning (440Hz) if scores are very close
        // This avoids false positives when track tuning is ambiguous
        if (fa.frequencyHz == kStandardTuningFrequency && score <= bestScore + 10) {
            // 440Hz gets a small advantage
            score -= 5;
        }

        // Update best if this frequency has a better score
        // or same score but more common frequency
        if (score < bestScore ||
                (score == bestScore && hasValidStart && !bestHasValidStart)) {
            bestScore = score;
            bestFrequency = fa.frequencyHz;
            bestHasValidStart = hasValidStart;
        }
    }

    // Require significant improvement over 440Hz to report a different frequency
    // This reduces false positives
    if (bestFrequency != kStandardTuningFrequency) {
        // Find the 440Hz score for comparison
        int score440 = INT_MAX;
        for (const FrequencyAnalyzer& fa : m_frequencyAnalyzers) {
            if (fa.frequencyHz == kStandardTuningFrequency) {
                score440 = static_cast<int>(fa.resultKeys.size()) * 100;
                if (fa.resultKeys.empty() ||
                        fa.resultKeys.front().first == mixxx::track::io::key::INVALID) {
                    score440 += 50;
                }
                break;
            }
        }

        // Require at least 20% improvement to report non-440Hz
        if (bestScore >= score440 * 0.8) {
            bestFrequency = kStandardTuningFrequency;
        }
    }

    return bestFrequency;
}

} // namespace mixxx
