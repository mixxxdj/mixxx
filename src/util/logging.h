#ifndef MIXXX_UTIL_LOGGING_H
#define MIXXX_UTIL_LOGGING_H

#include <QString>

namespace mixxx {

enum class LogLevel {
    Critical = 0,
    Warning = 1,
    Info = 2,
    Debug = 3,
    Default = Warning,
};

class Logging {
  public:
    // These are not thread safe. Only call them on Mixxx startup and shutdown.
    static void initialize(const QString& settingsPath,
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
    static bool debugEnabled() {
        return enabled(LogLevel::Debug);
    }

  private:
    Logging() = delete;

    static LogLevel s_logLevel;
};

}  // namespace mixxx

#endif /* MIXXX_UTIL_LOGGING_H */
