#pragma once

#include <QFlags>

namespace mixxx {

enum class LogLevel {
    Critical = 0,
    Warning = 1,
    Info = 2,
    Debug = 3,
    Trace = 4, // DEPRECATED (not available in Qt, used for profiling etc.)
};

enum class LogFlag {
    None = 0,
    LogToFile = 1,
    DebugAssertBreak = 1 << 1,
};
Q_DECLARE_FLAGS(LogFlags, LogFlag);
Q_DECLARE_OPERATORS_FOR_FLAGS(LogFlags);

/// Default log level for (console) logs.
constexpr LogLevel kLogLevelDefault = LogLevel::Warning;

/// Default log level for flushing the buffered log stream.
/// This is required to ensure that all buffered messages have
/// been written before Mixxx crashes.
constexpr LogLevel kLogFlushLevelDefault = LogLevel::Critical;

/// Utility class for accessing the logging settings that are
/// configured at startup.
class Logging {
  public:
    // These are not thread safe. Only call them on Mixxx startup and shutdown.
    static void initialize(
            const QString& logDirPath,
            LogLevel logLevel,
            LogLevel logFlushLevel,
            LogFlags flags);

    // Sets only the loglevel without the on-disk settings. Used by mixxx-test.
    static void setLogLevel(
            LogLevel logLevel) {
        s_logLevel = logLevel;
    }

    static void shutdown();

    static void flushLogFile();

    static bool shouldFlush(
            LogLevel logFlushLevel) {
        // Log levels are ordered by severity, i.e. more
        // severe log levels have a lower ordinal
        return s_logFlushLevel >= logFlushLevel;
    }

    static bool enabled(
            LogLevel logLevel) {
        return s_logLevel >= logLevel;
    }

  private:
    // Almost constant, i.e. initialized once at startup and
    // then could safely be read from multiple threads.
    static LogLevel s_logLevel;
    static LogLevel s_logFlushLevel;

    Logging() = delete;
};

} // namespace mixxx
