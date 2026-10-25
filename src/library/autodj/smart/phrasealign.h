#pragma once

#include <optional>
#include <vector>

/// Auto DJ 2.0 plus Video Mixing, phase 2: line a transition up with musical phrases.
///
/// Assumes 4/4 time and that the first beat of the beat grid is a downbeat
/// (bar 1). The tempo may bend (a beat map): all the maths counts beats. Phrases are counted in 8-bar blocks from
/// there. If Mixxx's beat grid does not start on a real downbeat, phrases
/// will be off by a whole number of beats; the DJ can move the grid.
/// Pure maths, no Mixxx types, so it can be unit-tested.
namespace phrasealign {

constexpr int kBeatsPerBar = 4;
constexpr int kBarsPerPhrase = 8;
constexpr int kBeatsPerPhrase = kBeatsPerBar * kBarsPerPhrase; // 32
/// How late a fade may still start (see plan()).
constexpr int kLateStartBeats = kBeatsPerBar;
/// Shortest fade of a mix that is not beatmatched: 1 bar (2 s at 120 BPM).
constexpr int kQuickSwitchBars = 1;

/// A beat grid, in seconds of the track (at its own speed, or real time at
/// a deck's speed: see atSpeed()). Either steady (first beat + beat length)
/// or a beat map that bends with the music (Mixxx's beat map, made when
/// "Assume constant tempo" is off): the time of every beat. Beats are
/// numbered from 0 (the first beat = start of bar 1); a fractional beat
/// number is a point between two beats. Before the first and after the last
/// beat of a map, the nearest beat length carries on.
struct Grid {
    double firstBeatSec = 0.0; ///< first beat of the grid = start of bar 1
    double beatSec = 0.0;      ///< length of one beat (a map: the average); 0 = no grid
    std::vector<double> beats; ///< a beat map: every beat (empty = steady grid)

    bool isValid() const {
        return beatSec > 0.0;
    }
    /// True for a beat map (the tempo may bend).
    bool isMap() const {
        return beats.size() >= 2;
    }
    /// Time of beat number `n` (0 = the first beat).
    double beatTime(double n) const;
    /// Beat number at time `sec` (fractional): the inverse of beatTime().
    double beatAt(double sec) const;
    /// Length of a beat around time `sec` (the middle one of the `span`
    /// beats before and after it). For a steady grid simply beatSec.
    double beatSecAt(double sec, int span = 4) const;
    /// Like beatSecAt, but not fooled by a beat map that goes wrong where
    /// the music stops (seen at the end of real songs: 120 BPM, then 159,
    /// then 235 BPM "beats" in the fade-out). The local beat counts when it
    /// is within 10% of the song's main tempo, or when it has held for 64
    /// beats (a real tempo change); otherwise the last one before it that
    /// does, or the main tempo.
    double steadyBeatSecAt(double sec) const;
    /// The same grid for a deck playing at `rateRatio` (1 = own speed),
    /// in real seconds.
    Grid atSpeed(double rateRatio) const;

    /// A beat map from the time of every beat (sorted; at least 2).
    static Grid fromBeats(std::vector<double> beatTimes);
};

/// What the transition should be.
struct Plan {
    double fromFadeBeginSec = 0.0; ///< outgoing track: fade starts (a phrase start)
    double fromFadeEndSec = 0.0;   ///< outgoing track: fade ends
    double toStartSec = 0.0;       ///< incoming track: starts here (a phrase start)
    int bars = 0;                  ///< length of the fade in bars
};

/// Where the main beat kicks in, as a beat number of `grid` (0 = first
/// beat). `bodyStartSec` is either measured (rough, 1 s steps: snapped to a
/// phrase start if one is close, otherwise the nearest bar) or `marked` by
/// the DJ (trusted: just the nearest beat). Auto DJ and the analyzer (which
/// sets the Intro End marker) both use this, so they always agree.
double entryBeat(const Grid& grid, double bodyStartSec, bool marked);

/// The last bar line at or before `bodyEndSec` (where the track starts to
/// fade), in seconds. Used for the automatic Outro Start marker.
double bodyEndBarSec(const Grid& grid, double bodyEndSec);

/// Fade length in bars for a wanted length in seconds: a whole number of
/// 8-bar phrases (at least one), measured at the outgoing track's tempo.
int barsForSeconds(double wantedSec, double beatSec);

/// Plans a phrase-aligned transition, or nullopt if it cannot be done
/// (no grid, or no phrase fits before `fromLimitSec`).
///
/// @param from outgoing track grid
/// @param to incoming track grid
/// @param fromNowSec where the outgoing track is now (the fade cannot start
///        earlier than this)
/// @param fromLimitSec the fade must be over by here (outro end / last sound)
/// @param toEarliestSec the incoming track should not start before this
///        (its intro start / first sound)
/// @param bars fade length in bars
/// @param toBodyStartSec where the incoming track's beat kicks in after its
///        intro (negative = unknown). Measured values are rough (1 s steps)
///        and get snapped to a nearby phrase or bar.
/// @param toBodyMarked true when the DJ marked the beat by hand (the Intro
///        End marker): it is trusted and only snapped to the nearest beat. The incoming track is started so its
///        beat kicks in at the MIDDLE of the fade, exactly where the bass
///        swaps and the crossfader has the incoming track at full volume:
///        the outgoing beat hands over straight to the incoming beat, with
///        no gap for the dancers. A long intro is partly skipped for this.
std::optional<Plan> plan(const Grid& from,
        const Grid& to,
        double fromNowSec,
        double fromLimitSec,
        double toEarliestSec,
        int bars,
        double toBodyStartSec = -1.0,
        bool toBodyMarked = false,
        double entryAt = 0.5);
/// `entryAt`: where in the fade the incoming beat kicks in: 0.5 = the
/// middle (at the bass swap, the classic Auto DJ 2.0 mix), 0 = the start
/// (the incoming song starts on its first downbeat, so its full sound
/// comes in gradually over the whole fade; its intro is skipped).

/// Fade Now: the "must be over by" limit that makes plan() start the fade
/// at the NEXT phrase start at least `minLeadSec` from now (time to cue the
/// incoming track). The wait is at most one phrase (8 bars).
double fadeNowLimitSec(const Grid& from, double nowSec, int bars, double minLeadSec = 2.0);

/// Like plan(), for a mix that is NOT beatmatched (tempos too far apart).
/// The outgoing fade is placed on its phrases the same way, but the
/// incoming beat kicks in as the fade ends instead of at its middle, so the
/// two different tempos never play their beats together. Its intro plays
/// under the end of the outgoing track. When the intro is shorter than the
/// fade, the fade is shortened to fit it (whole bars, at least
/// kQuickSwitchBars): a quick switch on the outgoing phrase ending instead
/// of two beats at different tempos on top of each other. `to` may have no
/// grid (then the body start is used as it is).
std::optional<Plan> planUnmatched(const Grid& from,
        const Grid& to,
        double fromNowSec,
        double fromLimitSec,
        double toEarliestSec,
        int bars,
        double toBodyStartSec = -1.0,
        bool toBodyMarked = false);

} // namespace phrasealign
