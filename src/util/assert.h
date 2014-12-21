#ifndef ASSERT_H
#define ASSERT_H

#include <QtDebug>

inline void mixxx_noop(void) {}

inline void mixxx_debug_assert(const char* assertion, const char* file, int line) {
#ifdef MIXXX_DEBUG_ASSERTIONS_FATAL
    qFatal("DEBUG ASSERT: \"%s\" in file %s, line %d", assertion, file, line);
#else
    qWarning("DEBUG ASSERT: \"%s\" in file %s, line %d", assertion, file, line);
#endif
}

inline bool mixxx_maybe_debug_assert_return_true(const char* assertion, const char* file, int line) {
#ifdef MIXXX_BUILD_DEBUG
    mixxx_debug_assert(assertion, file, line);
#endif
    return true;
}

inline void mixxx_release_assert(const char* assertion, const char* file, int line) {
    qFatal("ASSERT: \"%s\" in file %s, line %d", assertion, file, line);
}

// If cond is false, produces a fatal assertion and quits ungracefully. Think
// very hard before using this -- this should only be for the most dire of
// situations where we know Mixxx cannot take any action without potentially
// corrupting user data. Handle errors gracefully whenever possible.
#define RELEASE_ASSERT(cond) ((!(cond)) ? mixxx_release_assert(#cond, __FILE__, __LINE__) : mixxx_noop())

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
#define DEBUG_ASSERT(cond) ((!(cond)) ? mixxx_debug_assert(#cond, __FILE__, __LINE__) : mixxx_noop())
#else
#define DEBUG_ASSERT(cond)
#endif

#define DEBUG_ASSERT_AND_HANDLE(cond) if ((!(cond)) && mixxx_maybe_debug_assert_return_true(#cond, __FILE__, __LINE__))

#endif /* ASSERT_H */
