// this is a wapper around the fpclassify functions which prevents inlining 
// It is compiled without optization 
// The rest of the source of Mixxx is compiled with -ffast-math, which breaks 
// the fpclassify functions

#include <cmath>

#ifdef _MSC_VER
// VC++ uses _isnan() instead of isnan() and !_finite instead of isinf.
#include <float.h>
#define isnan(x) _isnan(x)
#define isinf(x) (!_finite(x))
#define fpclassify(x) _fpclass(x)
#define isfinite(x) _finite(x)
#else
// for isnan() and isinf() everywhere else use the cmath version. We define
// these as macros to prevent clashing with c++11 built-ins in the global
// namespace. If you say "using std::isnan;" then this will fail to build with
// std=c++11. See https://bugs.webkit.org/show_bug.cgi?id=59249 for some
// relevant discussion.
#define isnan std::isnan
#define isinf std::isinf
#define isnormal std::isnormal
#define fpclassify std::fpclassify
#define isfinite std::isfinite
#endif

int mixxx_fpclassify(float x) {
    return fpclassify(x);
}

int mixxx_isfinite(float x) {
    return isfinite(x);
}

int mixxx_isnormal(float x) {
    return isnormal(x);
}

int mixxx_isnan(float x) {
    return isnan(x);
}

int mixxx_isinf(float x) {
    return isinf(x); 
}

int mixxx_fpclassify(double x) {
    return fpclassify(x);
}

int mixxx_isfinite(double x) {
    return isfinite(x);
}

int mixxx_isnormal(double x) {
    return isnormal(x);
}

int mixxx_isnan(double x) {
    return isnan(x);
}

int mixxx_isinf(double x) {
    return isinf(x); 
}
