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

int util_isfinite(float x);
int util_isfinite(double x);

int util_isnormal(float x);
int util_isnormal(double x);

int util_isnan(float x);
int util_isnan(double x);

int util_isinf(float x);
int util_isinf(double x);
