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
//
// In order of Bug #1797746, since transition to qt5 it is needed unquote the
// output for mixxx.log with .noquote(), because in qt5 QDebug() is quoted by default.
#define controllerDebug(stream)       \
{                                     \
    if (ControllerDebug::enabled()) { \
        QDebug(QtDebugMsg).noquote() << ControllerDebug::kLogMessagePrefix << stream; \
    }                                 \
}                                     \

#endif // CONTROLLERDEBUG_H
