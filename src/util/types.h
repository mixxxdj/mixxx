#ifndef TYPES_H
#define TYPES_H

#include <climits>


// 16-bit integer sample data
typedef short int SAMPLE;
const SAMPLE SAMPLE_ZERO = 0;
// asymmetric range
const SAMPLE SAMPLE_MIN = SHRT_MIN;
const SAMPLE SAMPLE_MAX = SHRT_MAX;


// 32-bit floating-point sample data
typedef float CSAMPLE;
const CSAMPLE CSAMPLE_ZERO = 0.0f;
const CSAMPLE CSAMPLE_PEAK = 1.0f;
// symmetric range
const CSAMPLE CSAMPLE_MAX =  CSAMPLE_PEAK;
const CSAMPLE CSAMPLE_MIN = -CSAMPLE_PEAK;


#endif /* TYPES_H */
