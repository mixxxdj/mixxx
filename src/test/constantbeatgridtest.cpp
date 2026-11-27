#include <gtest/gtest.h>

#include <algorithm>
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

// A synthetic click track: onset detection function values, band magnitudes
// and tracker beats, all in onset frames.
struct ClickTrack {
    std::vector<double> onsets;
    std::vector<float> bands;
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
    track.bands.assign(size * mixxx::kQmOnsetBands, 0.01f);
    const double period = 60 * kOnsetFramesPerSec / bpm;
    const double maxJitter = 0.003 * kOnsetFramesPerSec;
    std::uniform_real_distribution<double> jitter(-maxJitter, maxJitter);
    for (double gridBeat = firstBeat; gridBeat < size - 6; gridBeat += period) {
        const double beat = gridBeat + jitter(generator);
        addBump(&track.onsets, beat, offBeatHiHats ? 0.6 : 1.0);
        const auto kickFrame = static_cast<std::size_t>(std::lround(beat));
        for (int band = 0; band < mixxx::kConstantBeatGridLowBands; ++band) {
            track.bands[kickFrame * mixxx::kQmOnsetBands + band] = 1.0f;
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

std::optional<mixxx::ConstantBeatGrid> refine(const ClickTrack& track) {
    return mixxx::refineConstantBeatGrid(
            track.onsets, track.bands, track.trackerBeats, kSampleRate, kStepSizeFrames);
}

TEST(ConstantBeatGridTest, ClickTrack) {
    for (const double bpm : {90.0, 128.0, 174.0}) {
        constexpr double kFirstBeat = 10.3;
        const ClickTrack track = makeClickTrack(bpm, kFirstBeat, false, false);
        const auto grid = refine(track);
        ASSERT_TRUE(grid) << bpm;
        EXPECT_DOUBLE_EQ(bpm, grid->bpm.value());
        EXPECT_NEAR(0, phaseError(*grid, kFirstBeat, bpm), 0.25) << bpm;
        // The first beat of the grid is within the first beat of the track.
        EXPECT_GE(grid->firstBeat.value(), 0);
        EXPECT_LT(grid->firstBeat.value(), 60 * kSampleRate / bpm);
    }
}

// A BPM a few ms of drift over the track away from a round one is rounded,
// one that drifts clearly away is not.
TEST(ConstantBeatGridTest, RoundsBpmWithinDrift) {
    constexpr double kFirstBeat = 10.3;
    for (const double bpm : {125.004, 125.03}) {
        const ClickTrack track = makeClickTrack(bpm, kFirstBeat, false, false);
        const auto grid = refine(track);
        ASSERT_TRUE(grid) << bpm;
        if (bpm == 125.004) {
            EXPECT_DOUBLE_EQ(125.0, grid->bpm.value());
        } else {
            EXPECT_NEAR(bpm, grid->bpm.value(), 0.005);
        }
    }
}

TEST(ConstantBeatGridTest, TrackerAtTwoThirdsOfBreakbeatTempo) {
    // A drum and bass pattern at 174 BPM: kicks on beats 1 and 3.5, snares on
    // beats 2 and 4 of each bar. The tracker follows every 1.5 beats (116 BPM).
    constexpr double kBpm = 174;
    constexpr double kFirstBeat = 5.4;
    const double period = 60 * kOnsetFramesPerSec / kBpm;
    const auto size = static_cast<std::size_t>(kTrackSecs * kOnsetFramesPerSec);
    std::mt19937 generator(1);
    std::uniform_real_distribution<double> noise(0, 0.2);
    std::vector<double> onsets(size);
    for (double& onset : onsets) {
        onset = noise(generator);
    }
    std::vector<float> bands(size * mixxx::kQmOnsetBands, 0.01f);
    const auto addKick = [&](double frame) {
        addBump(&onsets, frame, 1.0);
        const auto kickFrame = static_cast<std::size_t>(std::lround(frame));
        for (int band = 0; band < mixxx::kConstantBeatGridLowBands; ++band) {
            bands[kickFrame * mixxx::kQmOnsetBands + band] = 1.0f;
        }
    };
    for (double bar = kFirstBeat; bar + 4 * period < size - 6; bar += 4 * period) {
        addKick(bar);
        addBump(&onsets, bar + period, 1.0);
        addKick(bar + 2.5 * period);
        addBump(&onsets, bar + 3 * period, 1.0);
    }
    std::vector<double> trackerBeats;
    for (double beat = kFirstBeat; beat < size - 6; beat += 1.5 * period) {
        trackerBeats.push_back(std::round(beat));
    }
    const auto grid = mixxx::refineConstantBeatGrid(
            onsets, bands, trackerBeats, kSampleRate, kStepSizeFrames);
    ASSERT_TRUE(grid);
    EXPECT_DOUBLE_EQ(kBpm, grid->bpm.value());
    EXPECT_NEAR(0, phaseError(*grid, kFirstBeat, kBpm), 0.25);
}

TEST(ConstantBeatGridTest, OffBeatHiHats) {
    // The detection function peaks higher on the off-beat hi-hats, where the
    // tracker put its beats, but the kicks in the low bands are on the beat.
    constexpr double kBpm = 124;
    constexpr double kFirstBeat = 7.6;
    const ClickTrack track = makeClickTrack(kBpm, kFirstBeat, true, true);
    const auto grid = refine(track);
    ASSERT_TRUE(grid);
    EXPECT_DOUBLE_EQ(kBpm, grid->bpm.value());
    EXPECT_NEAR(0, phaseError(*grid, kFirstBeat, kBpm), 0.25);
}

TEST(ConstantBeatGridTest, OffBeatBassLine) {
    // Kicks on the beats peak in the detection function, a bass line on the
    // off-beats has the stronger low band onsets. The tracker follows the
    // kicks.
    constexpr double kBpm = 124;
    constexpr double kFirstBeat = 7.6;
    ClickTrack track = makeClickTrack(kBpm, kFirstBeat, false, false);
    const double period = 60 * kOnsetFramesPerSec / kBpm;
    std::fill(track.bands.begin(), track.bands.end(), 0.01f);
    const auto setLowBands = [&track](double frame, float magnitude) {
        const auto i = static_cast<std::size_t>(std::lround(frame));
        for (int band = 0; band < mixxx::kConstantBeatGridLowBands; ++band) {
            track.bands[i * mixxx::kQmOnsetBands + band] = magnitude;
        }
    };
    for (double beat = kFirstBeat; beat + period < track.onsets.size() - 6; beat += period) {
        setLowBands(beat, 0.2f);
        addBump(&track.onsets, beat + period / 2, 0.3);
        setLowBands(beat + period / 2, 1.0f);
    }
    const auto grid = refine(track);
    ASSERT_TRUE(grid);
    EXPECT_DOUBLE_EQ(kBpm, grid->bpm.value());
    EXPECT_NEAR(0, phaseError(*grid, kFirstBeat, kBpm), 0.25);
}

TEST(ConstantBeatGridTest, HalfTimeSnareOnHalfBeat) {
    // Half-time drum and bass: kicks on the beats at 87 BPM, louder snares on
    // the half beats with little low band energy. The tracker follows the
    // kicks.
    constexpr double kBpm = 87;
    constexpr double kFirstBeat = 7.6;
    ClickTrack track = makeClickTrack(kBpm, kFirstBeat, false, false);
    const double period = 60 * kOnsetFramesPerSec / kBpm;
    for (double beat = kFirstBeat; beat + period < track.onsets.size() - 6; beat += period) {
        addBump(&track.onsets, beat + period / 2, 1.8);
        const auto i = static_cast<std::size_t>(std::lround(beat + period / 2));
        for (int band = 0; band < mixxx::kConstantBeatGridLowBands; ++band) {
            track.bands[i * mixxx::kQmOnsetBands + band] = 0.3f;
        }
    }
    const auto grid = refine(track);
    ASSERT_TRUE(grid);
    EXPECT_DOUBLE_EQ(kBpm, grid->bpm.value());
    EXPECT_NEAR(0, phaseError(*grid, kFirstBeat, kBpm), 0.25);
}

TEST(ConstantBeatGridTest, TrackerOnOffBeatInHalfOfTheTrack) {
    // The tracker follows the off-beat, slightly early, in the middle half of
    // the track. The median offset of all its beats lies between the two
    // halves.
    constexpr double kBpm = 87;
    constexpr double kFirstBeat = 10.3;
    ClickTrack track = makeClickTrack(kBpm, kFirstBeat, false, false);
    if (track.trackerBeats.size() % 2 != 0) {
        track.trackerBeats.pop_back();
    }
    const double period = 60 * kOnsetFramesPerSec / kBpm;
    const std::size_t count = track.trackerBeats.size();
    for (std::size_t i = count / 4; i < count / 4 + count / 2; ++i) {
        track.trackerBeats[i] = std::round(track.trackerBeats[i] - 0.47 * period);
    }
    const auto grid = refine(track);
    ASSERT_TRUE(grid);
    EXPECT_DOUBLE_EQ(kBpm, grid->bpm.value());
    EXPECT_NEAR(0, phaseError(*grid, kFirstBeat, kBpm), 0.25);
}

TEST(ConstantBeatGridTest, TrackerAtHalfTempoInParts) {
    // The tracker follows every second beat in a long breakdown and in short
    // breaks between the other parts. The breakdown is the longest constant
    // region, but most of the track is tracked at the full tempo.
    constexpr double kBpm = 174;
    constexpr double kFirstBeat = 5.4;
    ClickTrack track = makeClickTrack(kBpm, kFirstBeat, false, false);
    const auto atHalfTempo = [](double secs) {
        return (secs >= 25 && secs < 33) || (secs >= 58 && secs < 66) ||
                (secs >= 91 && secs < 151);
    };
    std::vector<double> trackerBeats;
    for (std::size_t i = 0; i < track.trackerBeats.size(); ++i) {
        const double beat = track.trackerBeats[i];
        if (i % 2 == 0 || !atHalfTempo(beat / kOnsetFramesPerSec)) {
            trackerBeats.push_back(beat);
        }
    }
    const auto grid = mixxx::refineConstantBeatGrid(track.onsets,
            track.bands,
            trackerBeats,
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
            track.bands,
            std::vector<double>{100},
            kSampleRate,
            kStepSizeFrames));
}

} // namespace
