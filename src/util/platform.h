#ifndef MIXXX_UTIL_PLATFORM_H
#define MIXXX_UTIL_PLATFORM_H

#if defined(__GNUC__) && !defined(__INTEL_COMPILER)
// Clang and GCC
#define M_ALIGN(x) __attribute__((aligned(x)))
#define M_RESTRICT __restrict__
#define M_PACK(statement) statement __attribute__((packed))
#define M_MUST_USE_RESULT __attribute__((warn_unused_result))
#define M_PREDICT_FALSE(x) (__builtin_expect(x, 0))
#define M_PREDICT_TRUE(x) (__builtin_expect(!!(x), 1))
#elif defined(_MSC_VER)
// MSVC
#define M_ALIGN(x) __declspec(align(x))
#define M_RESTRICT __restrict
#define M_PACK(statement)  __pragma( pack(push, 1) ) statement __pragma( pack(pop) )
// MSVC supports _Check_return_ but it's a prefix, not a suffix. We could do
// something like M_PACK but for now don't bother.
#define M_MUST_USE_RESULT
#define M_PREDICT_FALSE(x) (x)
#define M_PREDICT_TRUE(x) (x)
#else
#error We do not support your compiler. Please email mixxx-devel@lists.sourceforge.net and tell us about your use case.
#endif

#if defined(__clang__) && defined(__has_warning)
#if __has_feature(cxx_attributes) && __has_warning("-Wimplicit-fallthrough")
#define M_FALLTHROUGH_INTENDED [[clang::fallthrough]]
#endif
#elif defined(__GNUC__) && __GNUC__ >= 7
// Taken from https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html#index-Wimplicit-fallthrough_003d
// We could also use a comment, but that would require ccache users to set the
// keep_comments_cpp option. If we switch to C++17, we can use [[fallthough]].
#define M_FALLTHROUGH_INTENDED __attribute__ ((fallthrough));
#endif

#ifndef M_FALLTHROUGH_INTENDED
#define M_FALLTHROUGH_INTENDED \
  do {                         \
  } while (0)
#endif

// Translate the MSVC _M_AMD64, _M_X64 and _M_IX86_FP to gcc style defines for SSE and SSE2 instruction sets
#if defined(_M_AMD64) || defined(_M_X64)
#define __SSE__
#define __SSE2__
#elif defined(_M_IX86_FP)
#if _M_IX86_FP >= 1
#define __SSE__
#endif
#if _M_IX86_FP == 2
#define __SSE2__
#endif
#endif

#endif /* MIXXX_UTIL_PLATFORM_H */
