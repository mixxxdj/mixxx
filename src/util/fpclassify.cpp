// this is a wrapper around the fpclassify functions which prevents inlining
// It is compiled without optimization
// The rest of the source of Mixxx is compiled with -ffast-math, which breaks
// the fpclassify functions

#ifdef __FAST_MATH__
#error This file must be compiled without a -ffast-math flag set
#endif

#include <cmath>

int util_fpclassify(float x) {
    return std::fpclassify(x);
}

bool util_isfinite(float x) {
    return std::isfinite(x);
}

bool util_isnormal(float x) {
    return std::isnormal(x);
}

int util_isnan(float x) {
    return std::isnan(x);
}

int util_isinf(float x) {
    return std::isinf(x);
}

int util_fpclassify(double x) {
    return std::fpclassify(x);
}

bool util_isfinite(double x) {
    return std::isfinite(x);
}

bool util_isnormal(double x) {
    return std::isnormal(x);
}

int util_isnan(double x) {
    return std::isnan(x);
}

int util_isinf(double x) {
    return std::isinf(x);
}
