
#ifndef DENORMALSAREZERO_H
#define DENORMALSAREZERO_H

// This was copied from the gcc header pmmintrin.h which requires SSE3
// According to https://gcc.gnu.org/bugzilla/show_bug.cgi?id=21408
// the DAZ bit is also available on SSE2
// There is probably a crasher risk for "Initial steppings of Pentium® 4
// processors" which did not support DAZ
// But we take the risk, since if it crashes, it will crash immediately after
// start. Effected users can switch to a legacy i386 build.
// See: https://software.intel.com/en-us/articles/x87-and-sse-floating-point-assists-in-ia-32-flush-to-zero-ftz-and-denormals-are-zero-daz

#ifdef __SSE__

#include <xmmintrin.h>

/* Additional bits in the MXCSR.  */
#define _MM_DENORMALS_ZERO_MASK     0x0040
#define _MM_DENORMALS_ZERO_ON       0x0040
#define _MM_DENORMALS_ZERO_OFF      0x0000

#define _MM_SET_DENORMALS_ZERO_MODE(mode) \
  _mm_setcsr ((_mm_getcsr () & ~_MM_DENORMALS_ZERO_MASK) | (mode))
#define _MM_GET_DENORMALS_ZERO_MODE() \
  (_mm_getcsr() & _MM_DENORMALS_ZERO_MASK)

#else

// this section is active on armhf builds, where DAZ is default
// https://gcc.gnu.org/onlinedocs/gcc/ARM-Options.html
// and for legacy i386 builds which are not recommended anyway

#define _MM_SET_DENORMALS_ZERO_MODE(mode)
#define _MM_GET_DENORMALS_ZERO_MODE()

#endif

#endif /* DENORMALSAREZERO_H */
