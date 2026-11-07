// Compatibility shims for linking the 2.6 vcpkg buildenv on newer glibc hosts.
//
// SoundTouch in the buildenv is built on the glibc-2.28 Rocky 8 floor with
// -ffast-math, so it references the legacy __exp_finite/__exp2_finite symbols.
// glibc >= 2.31 hides those symbols (keeping them only as compat aliases), so
// linking the buildenv on a newer host, e.g. Ubuntu 22.04, fails with an
// undefined reference.  Define them as thin wrappers around exp()/exp2(), the
// same aliasing glibc itself uses in libm-alias-finite.h.
//
// On glibc >= 2.31 the math headers no longer map exp() to __exp_finite under
// -ffast-math, so the wrappers call the real functions and do not recurse.
// The include guards make the file a no-op everywhere else, and the CMake side
// gates it on the vcpkg buildenv so this can be dropped once that buildenv is
// retired.
#ifdef __linux__
#include <features.h>
#include <math.h>

#if defined(__GLIBC_PREREQ)
#if __GLIBC_PREREQ(2, 31)

double __exp_finite(double x) {
    return exp(x);
}

double __exp2_finite(double x) {
    return exp2(x);
}

#endif // __GLIBC_PREREQ(2, 31)
#endif // defined(__GLIBC_PREREQ)
#endif // __linux__
