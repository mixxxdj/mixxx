// this is a wapper around the fpclassify functions which prevents inlining 
// It is compiled without optization 
// The rest of the source of Mixxx is compiled with -ffast-math, which breaks 
// the fpclassify functions

#include <cmath>

int util_fpclassify(float x) {
    return std::fpclassify(x);
}

int util_isfinite(float x) {
    return std::isfinite(x);
}

int util_isnormal(float x) {
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

int util_isfinite(double x) {
    return std::isfinite(x);
}

int util_isnormal(double x) {
    return std::isnormal(x);
}

int util_isnan(double x) {
    return std::isnan(x);
}

int util_isinf(double x) {
    return std::isinf(x); 
}
