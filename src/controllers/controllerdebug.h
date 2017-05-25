#ifndef CONTROLLERDEBUG_H
#define CONTROLLERDEBUG_H

#include <QDebug>


// Specifies whether or not we should dump incoming data to the console at
// runtime. This is useful for end-user debugging and script-writing.
class ControllerDebug {
  public:
    // Any debug statement starting with this prefix bypasses the --logLevel
    // command line flags.
    static constexpr const char* kLogMessagePrefix = "CDBG";

    static bool enabled();

    // Override the command-line argument (for testing)
    static void enable() {
        s_enabled = true;
    }

  private:
    ControllerDebug() = delete;

    static bool s_enabled;
};

// Usage: controllerDebug("hello" << "world");
//
// We prefix every log message with Logging::kControllerDebugPrefix so that we
// can bypass the --logLevel commandline argument when --controllerDebug is
// specified.
#define controllerDebug(stream)       \
{                                     \
    if (ControllerDebug::enabled()) { \
        QDebug(QtDebugMsg) << ControllerDebug::kLogMessagePrefix << stream; \
    }                                 \
}                                     \

#endif // CONTROLLERDEBUG_H
