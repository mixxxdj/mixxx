#include <gtest/gtest.h>

#include <cmath>
#include <random>
#include <vector>

#include "analyzer/plugins/constantbeatgrid.h"

namespace {

constexpr mixxx::audio::SampleRate kSampleRate(44100);
constexpr int kStepSizeFrames = 512;
constexpr double kOnsetFramesPerSec = 44100.0 / kStepSizeFrames;
constexpr double kTrackSecs = 180;
// The grid is moved 2 ms later than the onset peaks.
constexpr double kBeatDelayOnsetFrames = 0.002 * kOnsetFramesPerSec;

// A synthetic click track: onset detection function values, low band
// magnitudes and tracker beats, all in onset frames.
struct ClickTrack {
    std::vector<double> onsets;
    std::vector<float> lowBands;
    std::vector<double> trackerBeats;
};

void addBump(std::vector<double>* pOnsets, double center, double height) {
    const int centerFrame = static_cast<int>(std::lround(center));
    for (int i = centerFrame - 4; i <= centerFrame + 4; ++i) {
        if (i >= 0 && i < static_cast<int>(pOnsets->size())) {
            const double distance = i - center;
            (*pOnsets)[i] += height * std::exp(-distance * distance / 2);
        }
    }
}

// Beats at firstBeat + k * period (in onset frames) with a kick (detection
// function peak plus low band energy) on every beat and optionally a hi-hat
// (a higher detection function peak without low band energy) on every
// off-beat, over a noise floor. Each onset is up to 3 ms early or late. The
// tracker beats are whole frames, on the off-beats if trackerOnOffBeat is set.
ClickTrack makeClickTrack(double bpm, double firstBeat, bool offBeatHiHats, bool trackerOnOffBeat) {
    ClickTrack track;
    const auto size = static_cast<std::size_t>(kTrackSecs * kOnsetFramesPerSec);
    // A noise floor, as between the onsets of real music.
    std::mt19937 generator(1);
    std::uniform_real_distribution<double> noise(0, 0.2);
    track.onsets.resize(size);
    for (double& onset : track.onsets) {
        onset = noise(generator);
    }
    track.lowBands.assign(size * mixxx::kConstantBeatGridLowBands, 0.01f);
    const double period = 60 * kOnsetFramesPerSec / bpm;
    const double maxJitter = 0.003 * kOnsetFramesPerSec;
    std::uniform_real_distribution<double> jitter(-maxJitter, maxJitter);
    for (double gridBeat = firstBeat; gridBeat < size - 6; gridBeat += period) {
        const double beat = gridBeat + jitter(generator);
        addBump(&track.onsets, beat, offBeatHiHats ? 0.6 : 1.0);
        const auto kickFrame = static_cast<std::size_t>(std::lround(beat));
        for (int band = 0; band < mixxx::kConstantBeatGridLowBands; ++band) {
            track.lowBands[kickFrame * mixxx::kConstantBeatGridLowBands + band] = 1.0f;
        }
        const double offBeat = gridBeat + period / 2 + jitter(generator);
        if (offBeatHiHats) {
            addBump(&track.onsets, offBeat, 1.0);
        }
        track.trackerBeats.push_back(std::round(trackerOnOffBeat ? offBeat : beat));
    }
    return track;
}

// Distance of the grid's first beat to the nearest expected beat: firstBeat
// (in onset frames) plus the 2 ms delay, every 60 / bpm seconds. In onset
// frames; onset frame i is centred on sample i * kStepSizeFrames +
// kStepSizeFrames / 2.
double phaseError(const mixxx::ConstantBeatGrid& grid, double firstBeat, double bpm) {
    const double period = 60 * kOnsetFramesPerSec / bpm;
    const double expected = firstBeat + kBeatDelayOnsetFrames;
    const double actual = (grid.firstBeat.value() - kStepSizeFrames / 2.0) / kStepSizeFrames;
    double error = std::fmod(actual - expected, period);
    if (error > period / 2) {
        error -= period;
    } else if (error < -period / 2) {
        error += period;
    }
    return error;
}

TEST(ConstantBeatGridTest, ClickTrack) {
    for (const double bpm : {90.0, 128.0, 174.0}) {
        constexpr double kFirstBeat = 10.3;
        const ClickTrack track = makeClickTrack(bpm, kFirstBeat, false, false);
        const auto grid = mixxx::refineConstantBeatGrid(track.onsets,
                track.lowBands,
                track.trackerBeats,
                kSampleRate,
                kStepSizeFrames);
        ASSERT_TRUE(grid) << bpm;
        EXPECT_DOUBLE_EQ(bpm, grid->bpm.value());
        EXPECT_NEAR(0, phaseError(*grid, kFirstBeat, bpm), 0.25) << bpm;
        // The first beat of the grid is within the first beat of the track.
        EXPECT_GE(grid->firstBeat.value(), 0);
        EXPECT_LT(grid->firstBeat.value(), 60 * kSampleRate / bpm);
    }
}

TEST(ConstantBeatGridTest, OffBeatHiHats) {
    // The detection function peaks higher on the off-beat hi-hats, where the
    // tracker put its beats, but the kicks in the low bands are on the beat.
    constexpr double kBpm = 124;
    constexpr double kFirstBeat = 7.6;
    const ClickTrack track = makeClickTrack(kBpm, kFirstBeat, true, true);
    const auto grid = mixxx::refineConstantBeatGrid(track.onsets,
            track.lowBands,
            track.trackerBeats,
            kSampleRate,
            kStepSizeFrames);
    ASSERT_TRUE(grid);
    EXPECT_DOUBLE_EQ(kBpm, grid->bpm.value());
    EXPECT_NEAR(0, phaseError(*grid, kFirstBeat, kBpm), 0.25);
}

TEST(ConstantBeatGridTest, InvalidInput) {
    const ClickTrack track = makeClickTrack(120, 5, false, false);
    // Low bands not aligned with the onsets.
    EXPECT_FALSE(mixxx::refineConstantBeatGrid(track.onsets,
            std::vector<float>(track.onsets.size()),
            track.trackerBeats,
            kSampleRate,
            kStepSizeFrames));
    // Too few tracker beats.
    EXPECT_FALSE(mixxx::refineConstantBeatGrid(track.onsets,
            track.lowBands,
            std::vector<double>{100},
            kSampleRate,
            kStepSizeFrames));
}

} // namespace
