#ifndef MIXXX_UTIL_LOGGING_H
#define MIXXX_UTIL_LOGGING_H

#include <QDir>
#include <QFlags>

namespace mixxx {

enum class LogLevel {
    Critical = 0,
    Warning = 1,
    Info = 2,
    Debug = 3,
    Trace = 4, // for profiling etc.
};

enum class LogFlag {
    None = 0,
    LogToFile = 1,
    DebugAssertBreak = 1 << 1,
};
Q_DECLARE_FLAGS(LogFlags, LogFlag);
Q_DECLARE_OPERATORS_FOR_FLAGS(LogFlags);

constexpr LogLevel kLogLevelDefault = LogLevel::Warning;
constexpr LogLevel kLogFlushLevelDefault = LogLevel::Critical;

// Almost constant, i.e. initialized once at startup
// TODO(XXX): Remove this ugly "extern" hack after getting rid of
// the broken plugin architecture. Both globals should (again)
// become static members of the class Logging.
extern LogLevel g_logLevel;
extern LogLevel g_logFlushLevel;

class Logging {
  public:
    // These are not thread safe. Only call them on Mixxx startup and shutdown.
    static void initialize(
            const QDir& logDir,
            LogLevel logLevel,
            LogLevel logFlushLevel,
            LogFlags flags);

    // Sets only the loglevel without the on-disk settings.  Used by mixxx-test.
    static void setLogLevel(LogLevel logLevel);

    static void shutdown();

    static void flushLogFile();

    static bool enabled(LogLevel logLevel) {
        return g_logLevel >= logLevel;
    }
    static bool flushing(LogLevel logFlushLevel) {
        return g_logFlushLevel >= logFlushLevel;
    }
    static bool traceEnabled() {
        return enabled(LogLevel::Trace);
    }
    static bool debugEnabled() {
        return enabled(LogLevel::Debug);
    }
    static bool infoEnabled() {
        return enabled(LogLevel::Info);
    }

  private:
    Logging() = delete;
};

}  // namespace mixxx

#endif /* MIXXX_UTIL_LOGGING_H */
