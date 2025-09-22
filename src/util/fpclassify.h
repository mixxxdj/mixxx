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

bool util_isfinite(double x);
// Delete all other overloads to avoid accidental usage of them.
template<typename T>
bool util_isfinite(T x) = delete;

bool util_isnormal(double x);
// Delete all other overloads to avoid accidental usage of them.
template<typename T>
bool util_isnormal(T x) = delete;

int util_isnan(float x);
int util_isnan(double x);
// Delete all other overloads to avoid accidental usage of them.
template<typename T>
bool util_isnan(T x) = delete;

int util_isinf(float x);
int util_isinf(double x);
// Delete all other overloads to avoid accidental usage of them.
template<typename T>
bool util_isinf(T x) = delete;

// The following functions are only used in testing code.
// Don't use them in other -ffast-math code to avoid undefined behavior in
// floating-point arithmetic where the compiler assumes that arguments and
// results are not NaNs or +-Infs. For checking external librarie's return
// values use the appropiated function above.
float util_float_infinity();
double util_double_infinity();

float util_float_nan();
double util_double_nan();
