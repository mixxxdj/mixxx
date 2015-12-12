#ifndef UTIL_FPCLASSIFY_H
#define UTIL_FPCLASSIFY_H

#define isnan mixxx_isnan
#define isinf mixxx_isinf
#define isnormal mixxx_isnormal
#define fpclassify mixxx_fpclassify
#define isfinite mixxx_isfinite

int mixxx_fpclassify(float x);
int mixxx_isfinite(float x);
int mixxx_isnormal(float x);
int mixxx_isnan(float x);
int mixxx_isinf(float x);
int mixxx_fpclassify(double x);
int mixxx_isfinite(double x);
int mixxx_isnormal(double x);
int mixxx_isnan(double x);
int mixxx_isinf(double x);

#endif // UTIL_FPCLASSIFY_H
