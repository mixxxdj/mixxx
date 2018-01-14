#ifndef MIXXX_UTIL_LOGGING_H
#define MIXXX_UTIL_LOGGING_H

#include <QDir>


namespace mixxx {

enum class LogLevel {
    Critical = 0,
    Warning = 1,
    Info = 2,
    Debug = 3,
    Trace = 4, // for profiling etc.
};

constexpr LogLevel kLogLevelDefault = LogLevel::Warning;
constexpr LogLevel kFlushLevelDefault = LogLevel::Critical;

class Logging {
  public:
    // These are not thread safe. Only call them on Mixxx startup and shutdown.
    static void initialize(const QDir& settingsDir,
                           LogLevel logLevel,
                           LogLevel flushLevel,
                           bool debugAssertBreak);
    static void shutdown();

    static void flushLogFile();

    static bool enabled(LogLevel logLevel) {
        return s_logLevel >= logLevel;
    }
    static bool flushing(LogLevel flushLevel) {
        return s_flushLevel >= flushLevel;
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

    static LogLevel s_logLevel;
    static LogLevel s_flushLevel;
};

}  // namespace mixxx

#endif /* MIXXX_UTIL_LOGGING_H */
