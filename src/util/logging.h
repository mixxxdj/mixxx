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
    Default = Warning,
};

class Logging {
  public:
    // These are not thread safe. Only call them on Mixxx startup and shutdown.
    static void initialize(const QDir& settingsDir,
                           LogLevel logLevel,
                           bool debugAssertBreak);
    static void shutdown();

    // Query the current log level
    static LogLevel logLevel() {
        return s_logLevel;
    }
    static bool enabled(LogLevel logLevel) {
        return s_logLevel >= logLevel;
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
};

}  // namespace mixxx

#endif /* MIXXX_UTIL_LOGGING_H */
