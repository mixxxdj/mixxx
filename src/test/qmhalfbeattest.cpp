#include <gtest/gtest.h>

#include <QVector>
#include <algorithm>
#include <cmath>
#include <vector>

#include "analyzer/plugins/constantbeatgrid.h"
#include "analyzer/plugins/qmhalfbeat.h"

namespace {

constexpr mixxx::audio::SampleRate kSampleRate(44100);
constexpr int kStepSizeFrames = 512;
constexpr double kOnsetFramesPerSec = 44100.0 / kStepSizeFrames;

std::vector<double> makeBeats(double firstBeatSecs, double bpm, int count) {
    std::vector<double> beats;
    for (int i = 0; i < count; ++i) {
        beats.push_back((firstBeatSecs + i * 60 / bpm) * kSampleRate);
    }
    return beats;
}

struct KickHiHatTrack {
    std::vector<double> onsets;
    std::vector<float> bands;
};

// Kick drums on the beats, hi-hats half way between them. With offBeatKicks
// the off-beats are kicks as well, and the half beat is ambiguous.
KickHiHatTrack makeKickHiHatTrack(
        const std::vector<double>& beats, bool offBeatKicks = false) {
    const auto frameCount = static_cast<std::size_t>(
            (beats.back() / kSampleRate + 1) * kOnsetFramesPerSec);
    KickHiHatTrack track{std::vector<double>(frameCount, 0.1),
            std::vector<float>(frameCount * mixxx::kQmOnsetBands, 0.05f)};
    const auto addHit = [&track](double frame, int firstBand, int lastBand, double onset) {
        const auto start = static_cast<std::size_t>(std::lround(frame));
        for (std::size_t i = start; i < start + 6 && i < track.onsets.size(); ++i) {
            const double decay = std::exp(-0.7 * static_cast<double>(i - start));
            if (i == start) {
                track.onsets[i] += onset;
            }
            for (int band = firstBand; band < lastBand; ++band) {
                track.bands[i * mixxx::kQmOnsetBands + band] += static_cast<float>(5 * decay);
            }
        }
    };
    // Beat positions map to onset frames as (position - step / 2) / step.
    const auto onsetFrame = [](double position) {
        return (position - kStepSizeFrames / 2.0) / kStepSizeFrames;
    };
    for (std::size_t i = 0; i < beats.size(); ++i) {
        addHit(onsetFrame(beats[i]), 0, mixxx::kConstantBeatGridLowBands, 1.0);
        if (i + 1 < beats.size()) {
            const double offBeat = onsetFrame((beats[i] + beats[i + 1]) / 2);
            if (offBeatKicks) {
                addHit(offBeat, 0, mixxx::kConstantBeatGridLowBands, 1.0);
            } else {
                addHit(offBeat, mixxx::kQmOnsetBands - 2, mixxx::kQmOnsetBands, 2.0);
            }
        }
    }
    return track;
}

std::optional<mixxx::ConstantBeatGrid> checkHalfBeat(
        const mixxx::ConstantBeatGrid& grid, const KickHiHatTrack& track) {
    return mixxx::checkQmConstantGridHalfBeat(
            grid, track.onsets, track.bands, kSampleRate, kStepSizeFrames);
}

TEST(QmHalfBeatTest, ScoresHiHatsAsOffBeats) {
    constexpr int kBeatCount = 200;
    const KickHiHatTrack track = makeKickHiHatTrack(makeBeats(1.0, 120, kBeatCount));
    EXPECT_GT(mixxx::qmOffBeatScore(makeBeats(1.25, 120, kBeatCount - 1),
                      track.onsets,
                      track.bands,
                      kStepSizeFrames),
            mixxx::kQmMinOffBeatScoreToMoveGrid);
    EXPECT_LT(mixxx::qmOffBeatScore(makeBeats(1.0, 120, kBeatCount),
                      track.onsets,
                      track.bands,
                      kStepSizeFrames),
            0);
}

TEST(QmHalfBeatTest, MovesConstantGridOntoKicks) {
    constexpr int kBeatCount = 200;
    const KickHiHatTrack track = makeKickHiHatTrack(makeBeats(1.0, 120, kBeatCount));
    const mixxx::ConstantBeatGrid offBeatGrid{
            mixxx::Bpm(120), mixxx::audio::FramePos(0.25 * kSampleRate)};
    const auto grid = checkHalfBeat(offBeatGrid, track);
    ASSERT_TRUE(grid);
    EXPECT_EQ(mixxx::Bpm(120), grid->bpm);
    // Moved by half a beat onto the kicks at whole seconds.
    const double phaseSecs = std::fmod(grid->firstBeat.value() / kSampleRate, 0.5);
    EXPECT_LT(std::min(phaseSecs, 0.5 - phaseSecs), 0.001);
    // A grid on the kicks stays.
    const mixxx::ConstantBeatGrid onBeatGrid{
            mixxx::Bpm(120), mixxx::audio::FramePos(0.5 * kSampleRate)};
    EXPECT_EQ(onBeatGrid.firstBeat, checkHalfBeat(onBeatGrid, track)->firstBeat);
}

TEST(QmHalfBeatTest, KeepsConstantGridWhenHalfBeatIsAmbiguous) {
    constexpr int kBeatCount = 200;
    const KickHiHatTrack track = makeKickHiHatTrack(makeBeats(1.0, 120, kBeatCount), true);
    const mixxx::ConstantBeatGrid offBeatGrid{
            mixxx::Bpm(120), mixxx::audio::FramePos(0.25 * kSampleRate)};
    const auto grid = checkHalfBeat(offBeatGrid, track);
    ASSERT_TRUE(grid);
    EXPECT_EQ(offBeatGrid.firstBeat, grid->firstBeat);
}

TEST(QmHalfBeatTest, KeepsMissingGrid) {
    const KickHiHatTrack track = makeKickHiHatTrack(makeBeats(1.0, 120, 8));
    EXPECT_FALSE(mixxx::checkQmConstantGridHalfBeat(
            std::nullopt, track.onsets, track.bands, kSampleRate, kStepSizeFrames));
}

} // namespace
