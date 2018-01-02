#pragma once

#include <limits>

#include "util/math.h"


typedef double AnalyzerProgress;

constexpr AnalyzerProgress kAnalyzerProgressUnknown    = std::numeric_limits<AnalyzerProgress>::signaling_NaN();
constexpr AnalyzerProgress kAnalyzerProgressNone       = 0.0f;  //   0.0 %
constexpr AnalyzerProgress kAnalyzerProgressHalf       = 0.5f;  //  50.0 %
constexpr AnalyzerProgress kAnalyzerProgressFinalizing = 0.95f; //  95.0 %
constexpr AnalyzerProgress kAnalyzerProgressDone       = 1.0f;  // 100.0%

// Integer [0, 100]
inline
int analyzerProgressPercent(AnalyzerProgress analyzerProgress) {
    DEBUG_ASSERT(analyzerProgress >= kAnalyzerProgressNone);
    return int((100 * (math_min(analyzerProgress, kAnalyzerProgressDone) - kAnalyzerProgressNone)) /
            (kAnalyzerProgressDone - kAnalyzerProgressNone));
}
