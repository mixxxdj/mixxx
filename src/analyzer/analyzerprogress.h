#pragma once

#include "util/math.h"


// Measured in 0.1%, i.e. promille
constexpr int kAnalyzerProgressUnknown = -1;
constexpr int kAnalyzerProgressNone = 0; // 0.0 %
constexpr int kAnalyzerProgressFinalizing = 950; // 95.0 %
constexpr int kAnalyzerProgressDone = 1000; // 100.0%

// Integer [0, 100]
inline
int analyzerProgressPercent(int analyzerProgress) {
    DEBUG_ASSERT(analyzerProgress >= kAnalyzerProgressNone);
    return (100 * (math_min(analyzerProgress, kAnalyzerProgressDone) - kAnalyzerProgressNone)) /
            (kAnalyzerProgressDone - kAnalyzerProgressNone);
}

// Double [0.0, 1.0]
inline
double analyzerProgressDouble(int analyzerProgress) {
    DEBUG_ASSERT(analyzerProgress >= kAnalyzerProgressNone);
    return double(math_min(analyzerProgress, kAnalyzerProgressDone) - kAnalyzerProgressNone) /
            (kAnalyzerProgressDone - kAnalyzerProgressNone);
}
