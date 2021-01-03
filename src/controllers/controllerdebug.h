#pragma once

#include <QDebug>

#include "control/control.h"

// Specifies whether or not we should dump incoming data to the console at
// runtime. This is useful for end-user debugging and script-writing.
class ControllerDebug {
  public:
    // Any debug statement starting with this prefix bypasses the --logLevel
    // command line flags.
    static constexpr const char kLogMessagePrefix[] = "CDBG";
    static constexpr int kLogMessagePrefixLength = sizeof(kLogMessagePrefix) - 1;

    static bool isEnabled();

    /// Override the command-line argument (for testing)
    static void setEnabled(bool enabled) {
        s_enabled = enabled;
    }

    static void setTesting(bool isTesting) {
        s_testing = isTesting;
    }

    /// Return the appropriate flag for ControlProxies in mappings.
    ///
    /// Normally, accessing an invalid control from a mapping should *not*
    /// throw a debug assertion because controller mappings are considered
    /// user data. If we're testing or controller debugging is enabled, we *do*
    /// want assertions to prevent overlooking bugs in controller mappings.
    static ControlFlags controlFlags() {
        if (s_enabled || s_testing) {
            return ControlFlag::None;
        }

        return ControlFlag::AllowMissingOrInvalid;
    }

  private:
    ControllerDebug() = delete;

    static bool s_enabled;
    static bool s_testing;
};

// Usage: controllerDebug("hello" << "world");
//
// We prefix every log message with Logging::kControllerDebugPrefix so that we
// can bypass the --logLevel commandline argument when --controllerDebug is
// specified.
//
// In order of Bug #1797746, since transition to qt5 it is needed unquote the
// output for mixxx.log with .noquote(), because in qt5 QDebug() is quoted by default.
#define controllerDebug(stream)                                                           \
    {                                                                                     \
        if (ControllerDebug::isEnabled()) {                                               \
            QDebug(QtDebugMsg).noquote() << ControllerDebug::kLogMessagePrefix << stream; \
        }                                                                                 \
    }
