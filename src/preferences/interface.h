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

enum class KeylockMode {
    LockOriginalKey,
    LockCurrentKey
};

enum class KeyunlockMode {
    ResetLockedKey,
    KeepLockedKey
};

enum class LoadWhenDeckPlaying {
    Reject,
    Allow,
    AllowButStopDeck
};
