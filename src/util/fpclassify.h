#pragma once

#ifdef _MSC_VER

// VC++ uses _isnan() instead of isnan() and !_finite instead of isinf.
#include <float.h>
#define isnan(x) _isnan(x)
#define isinf(x) (!_finite(x))

#else

// We define 
// these as macros to prevent clashing with c++11 built-ins in the global
// namespace. If you say "using std::isnan;" then this will fail to build with
// std=c++11. See https://bugs.webkit.org/show_bug.cgi?id=59249 for some
// relevant discussion.

#define isnan util_isnan
#define isinf util_isinf
#define isnormal util_isnormal
#define fpclassify util_fpclassify
#define isfinite util_isfinite

int util_fpclassify(float x);
int util_isfinite(float x);
int util_isnormal(float x);
int util_isnan(float x);
int util_isinf(float x);
int util_fpclassify(double x);
int util_isfinite(double x);
int util_isnormal(double x);
int util_isnan(double x);
int util_isinf(double x);

#endif
