#pragma once

#include <optional>
#include <vector>

#include "audio/frame.h"
#include "audio/types.h"
#include "track/bpm.h"

namespace mixxx {

// A constant tempo beat grid: the beat at firstBeat and every 60 / bpm seconds
// before and after it.
struct ConstantBeatGrid {
    mixxx::Bpm bpm;
    mixxx::audio::FramePos firstBeat;

    double beatLengthFrames(mixxx::audio::SampleRate sampleRate) const {
        return 60.0 * sampleRate / bpm.value();
    }
    // The grid with its beats half way between the beats of this one.
    ConstantBeatGrid movedByHalfBeat(mixxx::audio::SampleRate sampleRate) const;
};

// Number of bands per onset detection frame that refineConstantBeatGrid() and
// makeQmBeatMap() expect: magnitude sums of the detection function's spectrum
// from these lower edges up to the next one, the last band up to the Nyquist
// frequency.
constexpr int kQmOnsetBands = 8;
constexpr double kQmOnsetBandEdgesHz[kQmOnsetBands] = {
        0, 60, 120, 250, 500, 1000, 2500, 6000};
// refineConstantBeatGrid() uses the first bands, 0-250 Hz.
constexpr int kConstantBeatGridLowBands = 3;
static_assert(kQmOnsetBandEdgesHz[kConstantBeatGridLowBands] == 250);

// The tempo step of refineConstantBeatGrid(): the BPM of the sharpest fold of
// onsets within 1 % of centerPeriod (in onset frames), rounded with
// BeatUtils::roundBpmWithinRange() as far as the beats drift by less than 25 ms
// over roundingLength (in onset frames), as BeatUtils::makeConstBpm() does.
// Returns an invalid BPM if none can be found.
mixxx::Bpm findRoundedOnsetBpm(const std::vector<double>& onsets,
        double centerPeriod,
        double framesPerMinute,
        double roundingLength);

// The alignment of refineConstantBeatGrid(): the phase (in onset frames) of
// the highest peak of the onsets folded at period within 5 frames of phase, to
// a fraction of a frame.
double alignPhaseToOnsets(const std::vector<double>& onsets, double phase, double period);

// Fits a constant tempo grid to a whole track, starting from the beats of the
// QM tempo tracker. It is used when the fixed tempo assumption is enabled, and
// by makeQmBeatMap() for tracks with a constant tempo.
//
// - Tempo: the start is the tempo of the longest constant region of the
//   tracker beats, or half or twice that if the tracker follows that for
//   longer in total. The onset detection function is folded over the whole
//   track at candidate periods around it. The period with the sharpest
//   fold wins, it is precise to a small fraction of a detection frame. If the
//   fold at 2/3 of that period is about as sharp, the tracker followed the
//   dotted tempo and the BPM is taken 3/2 higher. The BPM is then rounded with
//   findRoundedOnsetBpm().
// - Phase: the tracker's beats are averaged against the new grid, like
//   BeatUtils::adjustPhase(), modulo half a beat, since the tracker may
//   follow the off-beat in parts of the track.
// - Half beat: the broadband detection function often peaks on the off-beat
//   (hi-hats), while kick drums and bass sit on the beat. The grid is moved by
//   half a beat when the low band onsets plus the detection function are
//   stronger there; below 90 BPM the low bands weigh more, since the
//   detection function peaks at the snare on the half beat in half-time.
// - Timing: the beat is aligned to the highest peak of the folded detection
//   function within +-5 onset frames, then moved 2 ms later, the mean offset
//   to reference beat grids.
//
// onsets: detection function, one value per stepSizeFrames.
// bands: kQmOnsetBands magnitudes per onset frame, aligned with onsets.
// beats: tracker beats in onset frames (TempoTrackV2::calculateBeats).
// Returns nullopt if no grid can be found; the caller falls back to BeatUtils.
std::optional<ConstantBeatGrid> refineConstantBeatGrid(
        const std::vector<double>& onsets,
        const std::vector<float>& bands,
        const std::vector<double>& beats,
        mixxx::audio::SampleRate sampleRate,
        int stepSizeFrames);

} // namespace mixxx
