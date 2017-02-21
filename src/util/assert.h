#ifndef ASSERT_H
#define ASSERT_H

#include <QtDebug>

static constexpr const char* kDebugAssertPrefix = "DEBUG ASSERT";

inline void mixxx_noop(void) {}

inline void mixxx_debug_assert(const char* assertion, const char* file, int line, const char* function) {
    qCritical("%s: \"%s\" in function %s at %s:%d", kDebugAssertPrefix, assertion, function, file, line);
}

inline bool mixxx_maybe_debug_assert_return_true(const char* assertion, const char* file, int line, const char* function) {
#ifdef MIXXX_BUILD_DEBUG
    mixxx_debug_assert(assertion, file, line, function);
#else
    Q_UNUSED(assertion);
    Q_UNUSED(file);
    Q_UNUSED(line);
    Q_UNUSED(function);
#endif
    return true;
}

inline void mixxx_release_assert(const char* assertion, const char* file, int line, const char* function) {
    qFatal("ASSERT: \"%s\" in function %s at %s:%d", assertion, function, file, line);
}

// These macros provide the demangled function name (including helpful template
// type information) and are supported on every version of GCC, Clang, and MSVC
// that Mixxx supports.
#if defined(_MSC_VER)
#define ASSERT_FUNCTION __FUNCSIG__
#else
#define ASSERT_FUNCTION __PRETTY_FUNCTION__
#endif

// If cond is false, produces a fatal assertion and quits ungracefully. Think
// very hard before using this -- this should only be for the most dire of
// situations where we know Mixxx cannot take any action without potentially
// corrupting user data. Handle errors gracefully whenever possible.
#define RELEASE_ASSERT(cond) ((!(cond)) ? mixxx_release_assert(#cond, __FILE__, __LINE__, ASSERT_FUNCTION) : mixxx_noop())

// Checks that cond is true in debug builds. If cond is false then prints a
// warning message to the console. If Mixxx is built with
// MIXXX_DEBUG_ASSERTIONS_FATAL then the warning message is fatal. Compiles
// to nothing in release builds.
//
// Be careful of the common mistake with assertions:
//   DEBUG_ASSERT(doSomething());
//
// In release builds, doSomething() is never called!
#ifdef MIXXX_BUILD_DEBUG
#define DEBUG_ASSERT(cond) ((!(cond)) ? mixxx_debug_assert(#cond, __FILE__, __LINE__, ASSERT_FUNCTION) : mixxx_noop())
#else
#define DEBUG_ASSERT(cond)
#endif

#define VERIFY_OR_DEBUG_ASSERT(cond) if ((!(cond)) && mixxx_maybe_debug_assert_return_true(#cond, __FILE__, __LINE__, ASSERT_FUNCTION))

#endif /* ASSERT_H */
