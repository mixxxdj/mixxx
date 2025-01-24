#pragma once

/**
 * Functions for testing special floating-point values that
 * won't work with -ffast-math. This corresponding.cpp file
 * is compiled without -ffast-math and allows to invoke these
 * functions from -ffast-math code.
 *
 * NOTE: All usage of the corresponding std functions must be
 * avoided because they don't work as expected!!!
 */

int util_fpclassify(float x);
int util_fpclassify(double x);

bool util_isfinite(float x);
bool util_isfinite(double x);

bool util_isnormal(float x);
bool util_isnormal(double x);

int util_isnan(float x);
int util_isnan(double x);

int util_isinf(float x);
int util_isinf(double x);

// The following functions are only used in testing code.
// Don't use them in other -ffast-math code to avoid undefined behavior in
// floating-point arithmetic where the compiler assumes that arguments and
// results are not NaNs or +-Infs. For checking external librarie's return
// values use the appropiated function above.
float util_float_infinity();
double util_double_infinity();
