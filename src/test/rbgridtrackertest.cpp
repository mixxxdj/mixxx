#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <vector>

#include "analyzer/plugins/rbgrid/rbgridtracker.h"

namespace {

/// Stereo click track: a decaying 60 Hz "kick" on every beat plus a short
/// noise "hat" on every off-beat, first kick at firstBeatSec.
std::vector<float> makeTrack(double sampleRate, double bpm, double firstBeatSec, double seconds) {
    const auto frames = static_cast<std::size_t>(sampleRate * seconds);
    std::vector<float> out(frames * 2, 0.0f);
    const double period = 60.0 / bpm;
    unsigned noise = 1;
    for (int halfBeat = 0;; ++halfBeat) {
        const double t = firstBeatSec + halfBeat * period / 2;
        if (t >= seconds) {
            break;
        }
        const bool kick = halfBeat % 2 == 0;
        const auto start = static_cast<std::size_t>(t * sampleRate);
        const auto len = static_cast<std::size_t>((kick ? 0.15 : 0.03) * sampleRate);
        for (std::size_t i = 0; i < len && start + i < frames; ++i) {
            const double tt = i / sampleRate;
            float s;
            if (kick) {
                s = static_cast<float>(0.8 * std::exp(-tt * 25) * std::sin(2 * M_PI * 60 * tt));
            } else {
                noise = noise * 1664525u + 1013904223u;
                s = static_cast<float>(0.05 * std::exp(-tt * 150) *
                        ((noise >> 8) / 8388608.0 - 1.0));
            }
            out[2 * (start + i)] += s;
            out[2 * (start + i) + 1] += s;
        }
    }
    return out;
}

rbgrid::Grid analyze(double sampleRate,
        const std::vector<float>& samples,
        std::size_t chunk = 4096) { // analyzer-sized chunks by default
    rbgrid::Tracker tracker(sampleRate);
    for (std::size_t f = 0; f < samples.size() / 2; f += chunk) {
        tracker.processStereo(samples.data() + 2 * f, std::min(chunk, samples.size() / 2 - f));
    }
    return tracker.finalize();
}

void expectGrid(double sampleRate, double bpm, double firstBeatSec) {
    const rbgrid::Grid grid = analyze(sampleRate, makeTrack(sampleRate, bpm, firstBeatSec, 90.0));
    ASSERT_TRUE(grid.valid);
    EXPECT_DOUBLE_EQ(bpm, grid.bpm);
    const double period = 60.0 / bpm;
    // beat phase within 10 ms of the kicks
    double d = std::fmod(grid.firstBeatSec - firstBeatSec, period);
    if (d > period / 2) {
        d -= period;
    } else if (d < -period / 2) {
        d += period;
    }
    EXPECT_NEAR(0.0, d, 0.010);
    EXPECT_GE(grid.firstBeatSec, 0.0);
    EXPECT_LT(grid.firstBeatSec, period);
    EXPECT_GE(grid.barPhase, 0);
    EXPECT_LT(grid.barPhase, 4);
    EXPECT_NEAR(grid.firstBeatSec + grid.barPhase * period, grid.downbeatSec, 1e-9);
}

TEST(RbGridTrackerTest, ClickTrack128At44100) {
    expectGrid(44100, 128.0, 0.25);
}

TEST(RbGridTrackerTest, ClickTrack174At48000) {
    expectGrid(48000, 174.0, 0.1);
}

TEST(RbGridTrackerTest, ClickTrack95At44100) {
    expectGrid(44100, 95.0, 0.4);
}

// two 2:1 decimation stages
TEST(RbGridTrackerTest, ClickTrack120At96000) {
    expectGrid(96000, 120.0, 0.3);
}

// no decimation
TEST(RbGridTrackerTest, ClickTrack128At22050) {
    expectGrid(22050, 128.0, 0.2);
}

// Below 22.05 kHz only the tempo is expected to be right: the beat timing is
// calibrated for 22.05 kHz and above.
TEST(RbGridTrackerTest, ClickTrack100At16000) {
    const rbgrid::Grid grid = analyze(16000, makeTrack(16000, 100.0, 0.3, 90.0));
    ASSERT_TRUE(grid.valid);
    EXPECT_DOUBLE_EQ(100.0, grid.bpm);
}

// The streaming decimators must not depend on how the input is chunked.
TEST(RbGridTrackerTest, ChunkSizeDoesNotMatter) {
    const auto samples = makeTrack(96000, 126.0, 0.15, 60.0);
    const rbgrid::Grid a = analyze(96000, samples);
    const rbgrid::Grid b = analyze(96000, samples, 7);
    ASSERT_TRUE(a.valid);
    EXPECT_EQ(a.bpm, b.bpm);
    EXPECT_EQ(a.firstBeatSec, b.firstBeatSec);
    EXPECT_EQ(a.barPhase, b.barPhase);
    EXPECT_EQ(a.downbeatSec, b.downbeatSec);
}

TEST(RbGridTrackerTest, SilenceIsInvalid) {
    EXPECT_FALSE(analyze(44100, std::vector<float>(44100 * 2 * 30, 0.0f)).valid);
}

// three decimation stages
TEST(RbGridTrackerTest, ClickTrack140At192000) {
    expectGrid(192000, 140.0, 0.05);
}

// near the upper tempo search limit
TEST(RbGridTrackerTest, ClickTrack198At44100) {
    expectGrid(44100, 198.0, 0.1);
}

TEST(RbGridTrackerTest, TooShortIsInvalid) {
    EXPECT_FALSE(analyze(44100, makeTrack(44100, 128.0, 0.25, 5.0)).valid);
}

} // namespace
