#pragma once

namespace TrackTime {
enum class DisplayMode {
    ELAPSED,
    REMAINING,
    ELAPSED_AND_REMAINING,
};

enum class DisplayFormat {
    TRADITIONAL,
    TRADITIONAL_COARSE,
    SECONDS,
    SECONDS_LONG,
    KILO_SECONDS,
    HECTO_SECONDS,
};
} // namespace TrackTime

enum class LoadWhenDeckPlaying {
    Reject,
    Allow,
    AllowButStopDeck
};
