#ifndef TYPES_H
#define TYPES_H

#include "util/math.h"

#include <climits>

// 16-bit integer sample data within the asymmetric
// range [SHRT_MIN, SHRT_MAX].
typedef short int SAMPLE;
const SAMPLE SAMPLE_ZERO = 0;
const SAMPLE SAMPLE_MIN = SHRT_MIN;
const SAMPLE SAMPLE_MAX = SHRT_MAX;

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
const CSAMPLE CSAMPLE_ZERO = 0.0f;
const CSAMPLE CSAMPLE_ONE = 1.0f;
const CSAMPLE CSAMPLE_PEAK = CSAMPLE_ONE;

// Limits the range of a CSAMPLE value to [-CSAMPLE_PEAK, CSAMPLE_PEAK].
inline
CSAMPLE CSAMPLE_clamp(CSAMPLE in) {
    return math_clamp(in, -CSAMPLE_PEAK, CSAMPLE_PEAK);
}

// Gain values for weighted calculations of CSAMPLE
// data in the range [0.0, 1.0]. Same data type as
// CSAMPLE to avoid type conversions in calculations.
typedef CSAMPLE CSAMPLE_GAIN;
const float CSAMPLE_GAIN_ZERO = CSAMPLE_ZERO;
const float CSAMPLE_GAIN_ONE = CSAMPLE_ONE;
const float CSAMPLE_GAIN_MIN = CSAMPLE_GAIN_ZERO;
const float CSAMPLE_GAIN_MAX = CSAMPLE_GAIN_ONE;

// Limits the range of a CSAMPLE_GAIN value to [CSAMPLE_GAIN_MIN, CSAMPLE_GAIN_MAX].
inline
CSAMPLE_GAIN CSAMPLE_GAIN_clamp(CSAMPLE_GAIN in) {
    return math_clamp(in, CSAMPLE_GAIN_MIN, CSAMPLE_GAIN_MAX);
}

#endif /* TYPES_H */
