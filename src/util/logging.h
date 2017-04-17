#ifndef MIXXX_UTIL_LOGGING_H
#define MIXXX_UTIL_LOGGING_H

#include <QString>

namespace mixxx {

class Logging {
  public:
    enum class LogLevel {
        Critical = 0,
        Warning = 1,
        Info = 2,
        Debug = 3
    };
    static constexpr LogLevel kLogLevelDefault = LogLevel::Warning;
    // Any debug statement starting with this prefix bypasses the --logLevel
    // command line flags.
    static constexpr const char* kControllerDebugPrefix = "CDBG";

    // These are not thread safe. Only call them on Mixxx startup and shutdown.
    static void initialize(const QString& settingsPath,
                           LogLevel logLevel,
                           bool debugAssertBreak);
    static void shutdown();

  private:
    Logging() = delete;
};

}  // namespace mixxx

#endif /* MIXXX_UTIL_LOGGING_H */
