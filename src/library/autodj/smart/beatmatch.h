#pragma once

#include <optional>
#include <vector>

/// Auto DJ 2.0 plus Video Mixing, phase 2: the maths of a beatmatched transition.
/// Pure functions, no Mixxx controls, so they can be unit-tested.
namespace beatmatch {

/// Seconds the incoming track takes to glide back to its own tempo
/// after the mix. Slow enough that the change is not heard.
constexpr double kGlideSeconds = 30.0;

/// The tempo ratio to play the incoming track at so its beats line up with
/// the outgoing track (1.0 = its own speed). Uses half/double time when that
/// is closer (e.g. 72 BPM under 144 BPM). Returns nullopt when the change
/// would be more than `tolerancePct` percent (the DJ's 5% rule) or a BPM is
/// unknown; the transition is then a plain crossfade.
std::optional<double> matchRatio(double fromBpm, double toTrackBpm, double tolerancePct);

/// Ratio during the glide back: a straight line from `startRatio` to 1.0
/// over `glideSeconds`.
double glideRatio(double startRatio, double elapsedSeconds, double glideSeconds = kGlideSeconds);

/// Which bass (low EQ) is cut at a point in the crossfade (0..1).
/// Hard swap at the middle: first half the incoming bass is cut, second
/// half the outgoing bass is cut, so two basslines never play together.
struct BassState {
    bool fromLowKilled = false;
    bool toLowKilled = false;
};
BassState bassSwap(double progress);

/// Full EQ transition, on top of the bass swap: the mids and highs cross
/// over gradually instead of both tracks playing at full. First half: the
/// incoming mids/highs rise from kEqBlendFloor to full. Second half: the
/// outgoing mids/highs fall to kEqBlendFloor. The values multiply the
/// DJ's own EQ setting (1 = leave it as the DJ set it).
constexpr double kEqBlendFloor = 0.3; // about -10 dB
struct EqBlend {
    double fromMidHigh = 1.0;
    double toMidHigh = 1.0;
};
EqBlend eqBlend(double progress);

/// Stem mix: when both songs have their parts (drums, bass, other =
/// instrumental, vocals), the parts cross over instead of the EQ.
/// First half: the incoming instrumental rises, the outgoing vocals fall.
/// Middle: drums and bass swap hard, like the bass swap. Second half: the
/// outgoing instrumental falls, the incoming vocals rise. So two vocals
/// never play together and two basslines / kicks never clash. The values
/// multiply the DJ's own part levels (equal-power curves).
struct StemBlend {
    double fromVocals = 1.0;
    double fromInstrumental = 1.0;
    double fromDrums = 1.0;
    double fromBass = 1.0;
    double toVocals = 0.0;
    double toInstrumental = 0.0;
    double toDrums = 0.0;
    double toBass = 0.0;
};
/// `fadeDrums`: the drums cross over gradually instead (incoming drums rise
/// over the first half, outgoing drums fall over the second half); the bass
/// still swaps at the middle (two basslines clash).
StemBlend stemBlend(double progress, bool fadeDrums = false);

/// Singing detection: who sings during the mix, and when the outgoing
/// singer ends a line (as mix progress 0..1).
struct VocalPlan {
    bool fromSings = true;
    bool toSings = true;
    double swapAt = 0.5;
};
/// The stem mix with the vocals timed by the singing: a song that does not
/// sing during the mix keeps its vocals with its instrumental (untouched);
/// only when both sing are the vocals handed over, at `swapAt`, never
/// together.
StemBlend stemBlend(double progress, const VocalPlan& plan, bool fadeDrums = false);

/// Beat lock, for a track whose tempo bends (a beat map): during the mix
/// the incoming speed is set again and again so that its beats stay on the
/// outgoing beats, like a DJ riding the pitch fader.
///
/// @param fromBeat outgoing track: beat number now (fractional)
/// @param fromBeatRealSec outgoing track: length of its beat now, in real
///        seconds (at the deck's speed)
/// @param toBeat incoming track: beat number now (fractional)
/// @param toBeatTrackSec incoming track: length of its beat now, at its
///        own speed
/// @return the incoming tempo ratio: the one that gives it the same beat
///         length, nudged by up to kMaxLockNudge to pull a beat that has
///         slipped back in line within about kLockBeats beats. A slip of
///         less than kLockDeadBeats is left alone (no needless wobble).
constexpr double kLockBeats = 2.0;
constexpr double kMaxLockNudge = 0.06; // 6 % (key lock keeps the pitch)
constexpr double kLockDeadBeats = 0.01; // 5 ms at 120 BPM
/// Early in the mix (the incoming song still quiet), a slip bigger than
/// this is not pulled back slowly: the incoming song jumps in line.
constexpr double kResyncBeats = 0.05;
double followRatio(double fromBeat,
        double fromBeatRealSec,
        double toBeat,
        double toBeatTrackSec,
        double* pSlipBeats = nullptr);

/// Where the kick really starts after a song's beat lines. Beat analysis
/// does not put every song's lines at the same point of the kick: in real
/// songs the kick started 8 to 41 ms after the line. Two songs whose lines
/// are locked can then still have their kicks 30 ms apart (heard as a
/// clash), so beat lock lines up the kicks instead.
///
/// @param level the drum level (0..1), `rate` values per second (the
///        waveform: about 441 per second)
/// @param beatTimes the song's beat lines, in seconds
/// @return the middle offset in seconds (kick after the line = positive),
///         or nothing when fewer than 16 beats have a clear hit
std::optional<double> kickOffset(const std::vector<float>& level,
        double rate,
        const std::vector<double>& beatTimes);

} // namespace beatmatch
