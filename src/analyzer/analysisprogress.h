#pragma once

#include "util/math.h"


// Measured in 0.1%, i.e. promille
constexpr int kAnalysisProgressUnknown = -1;
constexpr int kAnalysisProgressNone = 0; // 0.0 %
constexpr int kAnalysisProgressFinalizing = 950; // 95.0 %
constexpr int kAnalysisProgressDone = 1000; // 100.0%

inline
bool analysisProgressValid(int analysisProgress) {
    return analysisProgress >= kAnalysisProgressNone;
}

// Integer [0, 100]
inline
int analysisProgressPercent(int analysisProgress) {
    DEBUG_ASSERT(analysisProgressValid(analysisProgress));
    return (100 * (math_min(analysisProgress, kAnalysisProgressDone) - kAnalysisProgressNone)) /
            (kAnalysisProgressDone - kAnalysisProgressNone);
}

// Double [0.0, 1.0]
inline
double analysisProgressDouble(int analysisProgress) {
    DEBUG_ASSERT(analysisProgressValid(analysisProgress));
    return double(math_min(analysisProgress, kAnalysisProgressDone) - kAnalysisProgressNone) /
            (kAnalysisProgressDone - kAnalysisProgressNone);
}
