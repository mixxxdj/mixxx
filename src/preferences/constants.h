#pragma once

namespace mixxx {

Q_NAMESPACE

// Don't change these constants since they are stored in user configuration
// files.
enum class TooltipsPreference {
    TOOLTIPS_OFF = 0,
    TOOLTIPS_ON = 1,
    TOOLTIPS_ONLY_IN_LIBRARY = 2,
};

// Settings to enable or disable the prevention to run the screensaver.
enum class ScreenSaverPreference {
    PREVENT_OFF = 0,
    PREVENT_ON = 1,
    PREVENT_ON_PLAY = 2
};

enum class MultiSamplingMode {
    Disabled = 0,
    Two = 2,
    Four = 4,
    Eight = 8,
    Sixteen = 16
};

Q_ENUM_NS(MultiSamplingMode);

}  // namespace mixxx
