// this is a wrapper around the fpclassify functions which prevents inlining
// It is compiled without optimization
// The rest of the source of Mixxx is compiled with -ffast-math, which breaks
// the fpclassify functions

#ifdef __FAST_MATH__
#error This file must be compiled without a -ffast-math flag set
#endif

#include <cmath>
#include <limits>

int util_isnan(float x) {
    return std::isnan(x);
}

int util_isinf(float x) {
    return std::isinf(x);
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

float util_float_infinity() {
    return std::numeric_limits<double>::infinity();
}

double util_double_infinity() {
    return std::numeric_limits<double>::infinity();
}

float util_float_nan() {
    return std::numeric_limits<double>::quiet_NaN();
}

double util_double_nan() {
    return std::numeric_limits<double>::quiet_NaN();
}
