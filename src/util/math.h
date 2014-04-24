#ifndef MATH_H
#define MATH_H

// Causes MSVC to define M_PI and friends.
// http://msdn.microsoft.com/en-us/library/4hwaceh6.aspx
#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>

#define math_max std::max
#define math_min std::min
#define math_max3(a, b, c) math_max(math_max((a), (b)), (c))

template <typename T>
inline T math_clamp(const T& value, const T& min, const T& max) {
    if (value > max) {
        return max;
    }
    if (value < min) {
        return min;
    }
    return value;
}

// NOTE(rryan): It is an error to call even() on a floating point number. Do not
// hack this to support floating point values! The programmer should be required
// to recognize to manually convert to avoid errors.
template <typename T>
inline bool even(const T& value) {
    return value % 2 == 0;
}

#ifdef _MSC_VER
// VC++ uses _isnan() instead of isnan() and !_finite instead of isinf.
#include <float.h>
#define isnan(x) _isnan(x)
#define isinf(x) (!_finite(x))
#else
// for isnan() everywhere else use cmath.h's version
using std::isnan;
using std::isinf;
#endif

inline int roundUpToPowerOf2(int v) {
    // From http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

#endif /* MATH_H */
