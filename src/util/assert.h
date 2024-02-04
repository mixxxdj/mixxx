#pragma once

#include <QtDebug>

#ifdef __OPTIMIZE__
#ifdef __has_builtin
#if __has_builtin(__builtin_constant_p)
#define HAS_BUILDIN_CONSTANT_P
#endif
#elif __GNUC__ < 10
#define HAS_BUILDIN_CONSTANT_P
#endif
#endif

static constexpr const char* kDebugAssertPrefix = "DEBUG ASSERT";

#ifdef MIXXX_DEBUG_ASSERTIONS_ENABLED
inline bool mixxx_debug_assert(const char* assertion,
        const char* file,
        int line,
        const char* function) {
    qCritical("%s: \"%s\" in function %s at %s:%d",
            kDebugAssertPrefix,
            assertion,
            function,
            file,
            line);
    return true;
}
#endif

inline void mixxx_release_assert(const char* assertion, const char* file, int line, const char* function) {
    qFatal("ASSERT: \"%s\" in function %s at %s:%d", assertion, function, file, line);
}

/// This function is intentionally not defined to produce a linker error
/// when the invocation is not dead code and optimized out
extern void link_assert_failed(void);
// Note void link_assert_failed(void) = delete; does not work, because
// it is evaluated before optimizing

// These macros provide the demangled function name (including helpful template
// type information) and are supported on every version of GCC, Clang, and MSVC
// that Mixxx supports.
#if defined(_MSC_VER)
#define ASSERT_FUNCTION __FUNCSIG__
#else
#define ASSERT_FUNCTION __PRETTY_FUNCTION__
#endif

/// If cond is false, produces a fatal assertion and quits ungracefully. Think
/// very hard before using this -- this should only be for the most dire of
/// situations where we know Mixxx cannot take any action without potentially
/// corrupting user data. Handle errors gracefully whenever possible.

#ifdef HAS_BUILDIN_CONSTANT_P
#define RELEASE_ASSERT(cond)                                                      \
    do                                                                            \
        if (Q_UNLIKELY(__builtin_constant_p(cond) && !static_cast<bool>(cond))) { \
            link_assert_failed();                                                 \
        } else if (Q_UNLIKELY(!static_cast<bool>(cond))) {                        \
            mixxx_release_assert(#cond, __FILE__, __LINE__, ASSERT_FUNCTION);     \
        }                                                                         \
    while (0)
#else
#define RELEASE_ASSERT(cond)                                                  \
    do                                                                        \
        if (!static_cast<bool>(cond)) [[unlikely]] {                          \
            mixxx_release_assert(#cond, __FILE__, __LINE__, ASSERT_FUNCTION); \
        }                                                                     \
    while (0)
#endif

/// Checks that cond is true in debug builds. If cond is false then prints a
/// warning message to the console. If Mixxx is built with
/// MIXXX_DEBUG_ASSERTIONS_FATAL then the warning message is fatal. Compiles
/// to nothing in release builds.
///
/// Be careful of the common mistake with assertions:
///   DEBUG_ASSERT(doSomething());
///
/// In release builds, doSomething() is never called!
#ifdef MIXXX_DEBUG_ASSERTIONS_ENABLED
#ifdef HAS_BUILDIN_CONSTANT_P
#define DEBUG_ASSERT(cond)                                                        \
    do                                                                            \
        if (Q_UNLIKELY(__builtin_constant_p(cond) && !static_cast<bool>(cond))) { \
            link_assert_failed();                                                 \
        } else if (Q_UNLIKELY(!static_cast<bool>(cond))) {                        \
            mixxx_debug_assert(#cond, __FILE__, __LINE__, ASSERT_FUNCTION);       \
        }                                                                         \
    while (0)
#else
#define DEBUG_ASSERT(cond)                                                  \
    do                                                                      \
        if (!static_cast<bool>(cond)) [[unlikely]] {                        \
            mixxx_debug_assert(#cond, __FILE__, __LINE__, ASSERT_FUNCTION); \
        }                                                                   \
    while (0)
#endif
#else
#define DEBUG_ASSERT(cond) \
    do {                   \
    } while (0)
#endif

#ifdef MIXXX_DEBUG_ASSERTIONS_ENABLED
#define X_DEBUG_ASSERT(cond)                                                \
    do                                                                      \
        if (Q_UNLIKELY(!static_cast<bool>(cond))) {                         \
            mixxx_debug_assert(#cond, __FILE__, __LINE__, ASSERT_FUNCTION); \
        }                                                                   \
    while (0)
#else
#define X_DEBUG_ASSERT(cond)
#endif

/// Same as DEBUG_ASSERT, but if MIXXX_DEBUG_ASSERTIONS_FATAL is disabled run
/// the specified fallback function. In most cases you should probably use
/// this rather than DEBUG_ASSERT. Only use DEBUG_ASSERT if there is no
/// appropriate fallback.
///
/// Use it like:
///   VERIFY_OR_DEBUG_ASSERT(value.is_valid()) { value = some_default; }
///
/// Note that the check and fallback will be included in release builds.
#ifdef MIXXX_DEBUG_ASSERTIONS_ENABLED
#ifdef HAS_BUILDIN_CONSTANT_P
#define VERIFY_OR_DEBUG_ASSERT(cond)                                          \
    if (Q_UNLIKELY(__builtin_constant_p(cond) && !static_cast<bool>(cond))) { \
        link_assert_failed();                                                 \
    } else if (Q_UNLIKELY(!static_cast<bool>(cond)) &&                        \
            mixxx_debug_assert(#cond, __FILE__, __LINE__, ASSERT_FUNCTION))
#else
#define VERIFY_OR_DEBUG_ASSERT(cond)            \
    if (Q_UNLIKELY(!static_cast<bool>(cond)) && \
            mixxx_debug_assert(#cond, __FILE__, __LINE__, ASSERT_FUNCTION))
#endif
#else
#define VERIFY_OR_DEBUG_ASSERT(cond) if (!static_cast<bool>(cond)) [[unlikely]]
#endif

#ifdef MIXXX_DEBUG_ASSERTIONS_ENABLED
#define X_VERIFY_OR_DEBUG_ASSERT(cond)          \
    if (Q_UNLIKELY(!static_cast<bool>(cond)) && \
            mixxx_debug_assert(#cond, __FILE__, __LINE__, ASSERT_FUNCTION))
#else
#define X_VERIFY_OR_DEBUG_ASSERT(cond) if (Q_UNLIKELY(!static_cast<bool>(cond)))
#endif

/// Same as DEBUG_ASSERT, but if MIXXX_DEBUG_ASSERTIONS_FATAL is disabled run
/// the specified fallback function. In most cases you should probably use this
/// rather than DEBUG_ASSERT. Only use DEBUG_ASSERT if there is no appropriate
/// fallback.
#ifdef MIXXX_DEBUG_ASSERTIONS_ENABLED
#define DEBUG_ASSERT_UNREACHABLE(cond) \
    mixxx_debug_assert(#cond, __FILE__, __LINE__, ASSERT_FUNCTION)
#else
#define DEBUG_ASSERT_UNREACHABLE(cond)
#endif
