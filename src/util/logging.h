#ifndef MIXXX_UTIL_LOGGING_H
#define MIXXX_UTIL_LOGGING_H

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

    static void initialize();
    static void shutdown();
  private:
    Logging() = delete;
};

void install_message_handler();

}  // namespace mixxx

#endif /* MIXXX_UTIL_LOGGING_H */
