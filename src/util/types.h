#pragma once

#include <cstddef>
#include <climits>

#include "util/math.h"

// Signed integer type for POT array indices, sizes and pointer
// arithmetic. Its size (32-/64-bit) depends on the CPU architecture.
// This should be used for all CSAMLE operations since it is fast and
// allows compiler auto vectorizing. For Qt container operations use
// just int as before.
typedef std::ptrdiff_t SINT;

// 16-bit integer sample data within the asymmetric
// range [SHRT_MIN, SHRT_MAX].
typedef short int SAMPLE;
constexpr SAMPLE SAMPLE_ZERO = 0;
constexpr SAMPLE SAMPLE_MIN = SHRT_MIN;
constexpr SAMPLE SAMPLE_MAX = SHRT_MAX;

// Limits the range of a SAMPLE value to [SAMPLE_MIN, SAMPLE_MAX].
inline
SAMPLE SAMPLE_clamp(SAMPLE in) {
    return math_clamp(in, SAMPLE_MIN, SAMPLE_MAX);
}

// Limits the range of a SAMPLE value to [-SAMPLE_MAX, SAMPLE_MAX].
inline
SAMPLE SAMPLE_clampSymmetric(SAMPLE in) {
    return math_clamp(in, static_cast<SAMPLE>(-SAMPLE_MAX), SAMPLE_MAX);
}

// 32-bit single precision floating-point sample data
// normalized within the range [-1.0, 1.0] with a peak
// amplitude of 1.0. No min/max constants here to
// emphasize the symmetric value range of CSAMPLE
// data!
typedef float CSAMPLE;
constexpr CSAMPLE CSAMPLE_ZERO = 0.0f;
constexpr CSAMPLE CSAMPLE_ONE = 1.0f;
constexpr CSAMPLE CSAMPLE_PEAK = CSAMPLE_ONE;

// Limits the range of a CSAMPLE value to [-CSAMPLE_PEAK, CSAMPLE_PEAK].
inline
CSAMPLE CSAMPLE_clamp(CSAMPLE in) {
    return math_clamp(in, -CSAMPLE_PEAK, CSAMPLE_PEAK);
}

// Gain values for weighted calculations of CSAMPLE
// data in the range [0.0, 1.0]. Same data type as
// CSAMPLE to avoid type conversions in calculations.
typedef CSAMPLE CSAMPLE_GAIN;
constexpr float CSAMPLE_GAIN_ZERO = CSAMPLE_ZERO;
constexpr float CSAMPLE_GAIN_ONE = CSAMPLE_ONE;
constexpr float CSAMPLE_GAIN_MIN = CSAMPLE_GAIN_ZERO;
constexpr float CSAMPLE_GAIN_MAX = CSAMPLE_GAIN_ONE;

// Limits the range of a CSAMPLE_GAIN value to [CSAMPLE_GAIN_MIN, CSAMPLE_GAIN_MAX].
inline
CSAMPLE_GAIN CSAMPLE_GAIN_clamp(CSAMPLE_GAIN in) {
    return math_clamp(in, CSAMPLE_GAIN_MIN, CSAMPLE_GAIN_MAX);
}
