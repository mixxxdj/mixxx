#pragma once
#include <QtDebug>
#include <cassert>

/// We previously used a custom assert implementation, so this is kept for
/// backward compatibility. The static_cast to bool makes the compiler warn if
/// you accidently use the assignment operator instead of a comparison.
///
/// This also takes care of the issue that commas anywhere in condition that
/// are not protected by parentheses are interpreted as macro argument separators
/// (`assert` is a function-like macro).
#define DEBUG_ASSERT(cond) assert(static_cast<bool>(cond))

/// This also asserts that the expression evaluates to `true`, but in case the
/// assertion was disabled (by defining`NDEBUG`), the specified fallback code
/// is executed instead.
/// In most cases you should probably use this rather than DEBUG_ASSERT. Only
/// use DEBUG_ASSERT if there is no appropriate fallback.
#ifndef NDEBUG
#define VERIFY_OR_DEBUG_ASSERT(cond) \
    DEBUG_ASSERT(cond);              \
    if (false)
#define RELEASE_ASSERT(cond) assert(static_cast<bool>(cond))
#else
#if defined(_MSC_VER)
#define ASSERT_FUNCTION __FUNCSIG__
#else
#define ASSERT_FUNCTION __PRETTY_FUNCTION__
#endif

// Convert any macro argument to string
#define STRINGIFY_INTERNAL(x) #x
#define STRINGIFY(x) STRINGIFY_INTERNAL(x)

inline bool mixxx_debug_assertion_failed(bool cond,
        const char* expr,
        const char* file,
        int line,
        const char* func) {
    if (!cond) {
        qCritical("DEBUG_ASSERT: \"%s\" in function %s at %s:%d", expr, func, file, line);
        return true;
    }
    return false;
}

inline void mixxx_release_assertion_failed(bool cond,
        const char* expr,
        const char* file,
        int line,
        const char* func) {
    if (!cond) {
        qFatal("RELEASE_ASSERT: \"%s\" in function %s at %s:%d", expr, func, file, line);
        abort();
    }
}

#define VERIFY_OR_DEBUG_ASSERT(cond)                          \
    if (mixxx_debug_assertion_failed(static_cast<bool>(cond), \
                #cond,                                        \
                __FILE__,                                     \
                __LINE__,                                     \
                ASSERT_FUNCTION))

#define RELEASE_ASSERT(cond)                                \
    mixxx_release_assertion_failed(static_cast<bool>(cond), \
            #cond,                                          \
            __FILE__,                                       \
            __LINE__,                                       \
            ASSERT_FUNCTION)
#endif
