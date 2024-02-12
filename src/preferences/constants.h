#pragma once

namespace mixxx {

namespace preferences {

namespace constants {

// Don't change these constants since they are stored in user configuration
// files.
enum class Tooltips {
    Off = 0,
    On = 1,
    OnlyInLibrary = 2,
};

// Settings to enable or disable the prevention to run the screensaver.
enum class ScreenSaver {
    Off = 0,
    On = 1,
    OnPlay = 2
};

} // namespace constants
} // namespace preferences
} // namespace mixxx
