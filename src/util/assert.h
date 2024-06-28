#pragma once

#include <QtDebug>
#include <source_location>

static constexpr const char* kDebugAssertPrefix = "DEBUG ASSERT";

#ifdef MIXXX_DEBUG_ASSERTIONS_ENABLED
inline void mixxx_debug_assert(const char* assertion, const std::source_location& loc) {
    qCritical("%s: \"%s\" in function %s at %s:%d:%d",
            kDebugAssertPrefix,
            assertion,
            loc.function_name(),
            loc.file_name(),
            loc.line(),
            loc.column());
}
#endif

#ifdef MIXXX_DEBUG_ASSERTIONS_ENABLED
inline bool mixxx_debug_assert_return_true(const char* assertion, const std::source_location& loc) {
    mixxx_debug_assert(assertion, loc);
    return true;
}
#endif

inline void mixxx_release_assert(const char* assertion, const std::source_location& loc) {
    qFatal("ASSERT: \"%s\" in function %s at %s:%d:%d",
            assertion,
            loc.function_name(),
            loc.file_name(),
            loc.line(),
            loc.column());
}

/// If cond is false, produces a fatal assertion and quits ungracefully. Think
/// very hard before using this -- this should only be for the most dire of
/// situations where we know Mixxx cannot take any action without potentially
/// corrupting user data. Handle errors gracefully whenever possible.
#define RELEASE_ASSERT(cond)                                              \
    do                                                                    \
        if (!static_cast<bool>(cond)) [[unlikely]] {                      \
            mixxx_release_assert(#cond, std::source_location::current()); \
        }                                                                 \
    while (0)

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
#define DEBUG_ASSERT(cond)                                              \
    do                                                                  \
        if (!static_cast<bool>(cond)) [[unlikely]] {                    \
            mixxx_debug_assert(#cond, std::source_location::current()); \
        }                                                               \
    while (0)
#else
#define DEBUG_ASSERT(cond) \
    do {                   \
    } while (0)
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
#define VERIFY_OR_DEBUG_ASSERT(cond)            \
    if (Q_UNLIKELY(!static_cast<bool>(cond)) && \
            mixxx_debug_assert_return_true(#cond, std::source_location::current()))
#else
#define VERIFY_OR_DEBUG_ASSERT(cond) if (!static_cast<bool>(cond)) [[unlikely]]
#endif
