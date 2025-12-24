#pragma once

// required for Qt-Macros
#include <qobjectdefs.h>

namespace mixxx {

namespace preferences {

inline namespace constants {
Q_NAMESPACE

// In order for this Q_NAMESPACE to work, all members of the namespace must
// be declared here. see QTBUG-68611

// Don't change these constants since they are stored in user configuration
// files.
enum class Tooltips {
    Off = 0,
    On = 1,
    OnlyInLibrary = 2,
};
Q_ENUM_NS(Tooltips);

// Settings to enable or disable the prevention to run the screensaver.
enum class ScreenSaver {
    Off = 0,
    On = 1,
    OnPlay = 2
};
Q_ENUM_NS(ScreenSaver);

enum class MultiSamplingMode {
    Disabled = 0,
    Two = 2,
    Four = 4,
    Eight = 8,
    Sixteen = 16
};
Q_ENUM_NS(MultiSamplingMode);

enum class SoundHardwareTab {
    Output,
    Input
};
Q_ENUM_NS(SoundHardwareTab);

} // namespace constants
} // namespace preferences
} // namespace mixxx
