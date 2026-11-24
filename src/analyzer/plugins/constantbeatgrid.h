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
};

// Number of low frequency bands per onset detection frame that
// refineConstantBeatGrid() expects: magnitude sums over 0-60, 60-120 and
// 120-250 Hz of the detection function's spectrum.
constexpr int kConstantBeatGridLowBands = 3;
constexpr double kConstantBeatGridLowBandEdgesHz[kConstantBeatGridLowBands + 1] = {
        0, 60, 120, 250};

// Fits a constant tempo grid to a whole track, starting from the beats of the
// QM tempo tracker. It is used when the fixed tempo assumption is enabled.
//
// - Tempo: the onset detection function is folded over the whole track at
//   candidate periods around the tracker's tempo. The period with the sharpest
//   fold wins, it is precise to a small fraction of a detection frame. The BPM is
//   then rounded with BeatUtils::roundBpmWithinRange() within the range of
//   periods whose fold is nearly as sharp.
// - Phase: the tracker's beats are averaged against the new grid, like
//   BeatUtils::adjustPhase().
// - Half beat: the broadband detection function often peaks on the off-beat
//   (hi-hats), while kick drums and bass sit on the beat. The grid is moved by
//   half a beat when the low band onsets plus the detection function are
//   stronger there.
// - Timing: the beat is aligned to the nearest peak of the folded detection
//   function, then moved 2 ms later, the mean offset to rekordbox's beats.
//
// onsets: detection function, one value per stepSizeFrames.
// lowBands: kConstantBeatGridLowBands magnitudes per onset frame, aligned with onsets.
// beats: tracker beats in onset frames (TempoTrackV2::calculateBeats).
// Returns nullopt if no grid can be found; the caller falls back to BeatUtils.
std::optional<ConstantBeatGrid> refineConstantBeatGrid(
        const std::vector<double>& onsets,
        const std::vector<float>& lowBands,
        const std::vector<double>& beats,
        mixxx::audio::SampleRate sampleRate,
        int stepSizeFrames);

} // namespace mixxx
