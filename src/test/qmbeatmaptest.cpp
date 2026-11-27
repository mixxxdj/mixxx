#include <gtest/gtest.h>

#include <QVector>
#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

#include "analyzer/plugins/constantbeatgrid.h"
#include "analyzer/plugins/qmbeatmap.h"
#include "analyzer/plugins/qmhalfbeat.h"
#include "track/beatutils.h"

namespace {

constexpr mixxx::audio::SampleRate kSampleRate(44100);
constexpr int kStepSizeFrames = 512;
constexpr double kOnsetFramesPerSec = 44100.0 / kStepSizeFrames;

QVector<mixxx::audio::FramePos> makeBeats(double firstBeatSecs, double bpm, int count) {
    QVector<mixxx::audio::FramePos> beats;
    for (int i = 0; i < count; ++i) {
        beats.append(mixxx::audio::FramePos((firstBeatSecs + i * 60 / bpm) * kSampleRate));
    }
    return beats;
}

// The tracker beats of a 120 BPM track, with a few beats in the middle that
// are 40 ms late: the regions before and after have the same tempo and phase.
QVector<mixxx::audio::FramePos> makeWobblyBeats() {
    QVector<mixxx::audio::FramePos> beats = makeBeats(0.5, 120, 256);
    for (int i = 120; i < 124; ++i) {
        beats[i] += 0.040 * kSampleRate;
    }
    return beats;
}

TEST(QmBeatMapTest, MergeConstRegionsJoinsSplitRegions) {
    const QVector<mixxx::audio::FramePos> beats = makeWobblyBeats();
    const auto regions = BeatUtils::retrieveConstRegions(beats, kSampleRate);
    ASSERT_GT(regions.size(), 3);

    const auto merged = mixxx::qmbeatmap::mergeConstRegions(regions, beats, kSampleRate);
    // One region and the end marker.
    ASSERT_EQ(2, merged.size());
    EXPECT_NEAR(beats.first().value(), merged[0].firstBeat.value(), 1);
    EXPECT_NEAR(0.5 * kSampleRate, merged[0].beatLength, 0.1);
    EXPECT_EQ(beats.last(), merged[1].firstBeat);
}

TEST(QmBeatMapTest, MergeConstRegionsKeepsTempoChange) {
    // A tempo change after some unstable beats: the regions around them have
    // the same phase at the change but not the same tempo.
    QVector<mixxx::audio::FramePos> beats = makeBeats(0.5, 120, 128);
    beats += makeBeats(0.5 + 128 * 0.5 + 0.03, 100, 3);
    beats += makeBeats(0.5 + 131 * 0.5, 126, 128);
    const auto regions = BeatUtils::retrieveConstRegions(beats, kSampleRate);
    ASSERT_GT(regions.size(), 3);
    const auto merged = mixxx::qmbeatmap::mergeConstRegions(regions, beats, kSampleRate);
    EXPECT_EQ(regions.size(), merged.size());
}

TEST(QmBeatMapTest, MergeConstRegionsKeepsPhaseShift) {
    // The tracker jumps by half a beat after some unstable beats.
    QVector<mixxx::audio::FramePos> beats = makeBeats(0.5, 120, 128);
    beats += makeBeats(0.5 + 128 * 0.5 + 0.1, 100, 4);
    beats += makeBeats(0.5 + 128 * 0.5 + 0.1 + 4 * 0.6 + 0.25, 120, 128);
    const auto regions = BeatUtils::retrieveConstRegions(beats, kSampleRate);
    const auto merged = mixxx::qmbeatmap::mergeConstRegions(regions, beats, kSampleRate);
    EXPECT_EQ(regions.size(), merged.size());
}

// The detection function and band magnitudes of a 120 BPM track with a kick
// (low bands) on every beat and a louder hi-hat (high bands) on every
// off-beat.
struct KickHiHatTrack {
    std::vector<double> onsets;
    std::vector<float> bands;
};

// Kick drums on the beats, hi-hats half way between them. With offBeatKicks
// the off-beats are kicks as well, and the half beat is ambiguous.
KickHiHatTrack makeKickHiHatTrack(
        const QVector<mixxx::audio::FramePos>& beats, bool offBeatKicks = false) {
    const auto frameCount = static_cast<std::size_t>(
            (beats.last().value() / kSampleRate + 1) * kOnsetFramesPerSec);
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
    for (int i = 0; i < beats.size(); ++i) {
        addHit(onsetFrame(beats[i].value()), 0, mixxx::kConstantBeatGridLowBands, 1.0);
        if (i + 1 < beats.size()) {
            const double offBeat = onsetFrame((beats[i].value() + beats[i + 1].value()) / 2);
            if (offBeatKicks) {
                addHit(offBeat, 0, mixxx::kConstantBeatGridLowBands, 1.0);
            } else {
                addHit(offBeat, mixxx::kQmOnsetBands - 2, mixxx::kQmOnsetBands, 2.0);
            }
        }
    }
    return track;
}

double maxNearestDistance(const QVector<mixxx::audio::FramePos>& from,
        const QVector<mixxx::audio::FramePos>& to) {
    double maxDistance = 0;
    for (const auto beat : from) {
        double distance = 1e9;
        for (const auto other : to) {
            distance = std::min(distance, std::fabs(other - beat));
        }
        maxDistance = std::max(maxDistance, distance);
    }
    return maxDistance;
}

// The largest distance from a beat of one list to the nearest beat of the
// other, both ways: missing and extra beats count.
double maxDistanceSecs(const QVector<mixxx::audio::FramePos>& beatMap,
        const QVector<mixxx::audio::FramePos>& beats) {
    return std::max(maxNearestDistance(beats, beatMap), maxNearestDistance(beatMap, beats)) /
            kSampleRate;
}

mixxx::QmBeatMap beatMapOf(const QVector<mixxx::audio::FramePos>& beats,
        const KickHiHatTrack& track,
        const std::optional<mixxx::ConstantBeatGrid>& grid = std::nullopt) {
    return mixxx::makeQmBeatMap(
            beats, track.onsets, track.bands, kSampleRate, kStepSizeFrames, grid);
}

double offBeatScore(
        const QVector<mixxx::audio::FramePos>& beatMap, const KickHiHatTrack& track) {
    std::vector<double> beats;
    for (const auto beat : beatMap) {
        beats.push_back(beat.value());
    }
    return mixxx::qmOffBeatScore(beats, track.onsets, track.bands, kStepSizeFrames);
}

TEST(QmBeatMapTest, MovesBeatMapFromOffBeat) {
    constexpr int kBeatCount = 200;
    const KickHiHatTrack track = makeKickHiHatTrack(makeBeats(1.0, 120, kBeatCount));
    const QVector<mixxx::audio::FramePos> trackerBeats = makeBeats(1.25, 120, kBeatCount - 1);
    EXPECT_GT(offBeatScore(trackerBeats, track),
            0);

    const auto beatMap = beatMapOf(trackerBeats, track);
    EXPECT_FALSE(beatMap.constantGrid);
    // The kicks after the first tracker beat, up to the last kick.
    const QVector<mixxx::audio::FramePos> beats = makeBeats(1.5, 120, kBeatCount - 1);
    EXPECT_LT(maxDistanceSecs(beatMap.beats, beats), 0.002);
}

TEST(QmBeatMapTest, KeepsBeatMapOnBeat) {
    constexpr int kBeatCount = 200;
    const auto beats = makeBeats(1.0, 120, kBeatCount);
    const KickHiHatTrack track = makeKickHiHatTrack(beats);
    EXPECT_LT(offBeatScore(beats, track), 0);

    const auto beatMap = beatMapOf(beats, track);
    EXPECT_FALSE(beatMap.constantGrid);
    EXPECT_LT(maxDistanceSecs(beatMap.beats, beats), 0.002);
}

TEST(QmBeatMapTest, MovesConstantGridOntoKicks) {
    constexpr int kBeatCount = 200;
    const KickHiHatTrack track = makeKickHiHatTrack(makeBeats(1.0, 120, kBeatCount));
    // The tracker and the constant grid both follow the hi-hats.
    const QVector<mixxx::audio::FramePos> trackerBeats = makeBeats(1.25, 120, kBeatCount - 1);
    const mixxx::ConstantBeatGrid offBeatGrid{
            mixxx::Bpm(120), mixxx::audio::FramePos(0.25 * kSampleRate)};

    const auto beatMap = beatMapOf(trackerBeats, track, offBeatGrid);
    ASSERT_TRUE(beatMap.constantGrid);
    EXPECT_TRUE(beatMap.beats.isEmpty());
    EXPECT_EQ(mixxx::Bpm(120), beatMap.constantGrid->bpm);
    // Moved by half a beat onto the kicks at whole seconds.
    const double phaseSecs = std::fmod(beatMap.constantGrid->firstBeat.value() / kSampleRate, 0.5);
    EXPECT_LT(std::min(phaseSecs, 0.5 - phaseSecs), 0.001);
}

TEST(QmBeatMapTest, KeepsTempoChange) {
    QVector<mixxx::audio::FramePos> beats = makeBeats(1.0, 120, 128);
    beats += makeBeats(1.0 + 128 * 0.5, 126, 128);
    const KickHiHatTrack track = makeKickHiHatTrack(beats);
    const mixxx::ConstantBeatGrid grid{mixxx::Bpm(123), mixxx::audio::FramePos(0)};

    const auto beatMap = beatMapOf(beats, track, grid);
    EXPECT_FALSE(beatMap.constantGrid);
    EXPECT_LT(maxDistanceSecs(beatMap.beats, beats), 0.005);
}

TEST(QmBeatMapTest, KeepsConstantGridWhenTrackerFollowsTriplets) {
    // A 120 BPM track that plays triplets only from second 49 on: four hits
    // per three beats, 160 BPM. The tracker follows the triplets, and neither
    // grid agrees with the onsets of the other part, so the two parts stay
    // separate regions and the triplet part is the longer one.
    constexpr double kSwitchSecs = 1.0 + 96 * 0.5;
    constexpr int kTripletCount = 213;
    KickHiHatTrack track = makeKickHiHatTrack(makeBeats(kSwitchSecs, 160, kTripletCount));
    const KickHiHatTrack straight = makeKickHiHatTrack(makeBeats(1.0, 120, 97));
    for (std::size_t i = 0; i < std::min(track.onsets.size(), straight.onsets.size()); ++i) {
        track.onsets[i] += straight.onsets[i] - 0.1;
        for (int band = 0; band < mixxx::kQmOnsetBands; ++band) {
            const std::size_t index = i * mixxx::kQmOnsetBands + band;
            track.bands[index] += straight.bands[index] - 0.05f;
        }
    }
    QVector<mixxx::audio::FramePos> trackerBeats = makeBeats(1.0, 120, 96);
    trackerBeats += makeBeats(kSwitchSecs, 160, kTripletCount);
    const mixxx::ConstantBeatGrid grid{mixxx::Bpm(120), mixxx::audio::FramePos(kSampleRate)};

    const auto beatMap = beatMapOf(trackerBeats, track, grid);
    ASSERT_TRUE(beatMap.constantGrid);
    EXPECT_EQ(mixxx::Bpm(120), beatMap.constantGrid->bpm);
}

TEST(QmBeatMapTest, KeepsConstantGridOverBeatlessIntroAndOutro) {
    // 120 BPM kicks from second 30 to 150 of a 200 s track, only noise before
    // and after, where the tracker wanders at other tempos. The kicks cover
    // less than kConstantMinShare of the track, but the grid explains the
    // noise as well as the tracker's tempos do.
    const QVector<mixxx::audio::FramePos> kicks = makeBeats(30.0, 120, 240);
    KickHiHatTrack track = makeKickHiHatTrack(kicks);
    const auto frameCount = static_cast<std::size_t>(200 * kOnsetFramesPerSec);
    track.onsets.resize(frameCount, 0.1);
    track.bands.resize(frameCount * mixxx::kQmOnsetBands, 0.05f);
    std::mt19937 generator(1);
    std::uniform_real_distribution<double> noise(0, 0.3);
    for (double& onset : track.onsets) {
        onset += noise(generator);
    }
    QVector<mixxx::audio::FramePos> trackerBeats = makeBeats(0.4, 97, 47);
    trackerBeats += kicks;
    trackerBeats += makeBeats(150.3, 129, 106);
    const mixxx::ConstantBeatGrid grid{mixxx::Bpm(120), mixxx::audio::FramePos(0)};

    const auto beatMap = beatMapOf(trackerBeats, track, grid);
    ASSERT_TRUE(beatMap.constantGrid);
    EXPECT_EQ(mixxx::Bpm(120), beatMap.constantGrid->bpm);
}

TEST(QmBeatMapTest, SmoothsDriftInBreakdown) {
    // 64 beats of music, a quiet breakdown in which the tracker drifts away,
    // then 64 beats of music in the same grid.
    const QVector<mixxx::audio::FramePos> grid = makeBeats(1.0, 120, 160);
    KickHiHatTrack track = makeKickHiHatTrack(grid);
    const auto breakdownBegin = static_cast<std::size_t>(
            (grid[64].value() - kStepSizeFrames / 2.0) / kStepSizeFrames - 3);
    const auto breakdownEnd = static_cast<std::size_t>(
            (grid[96].value() - kStepSizeFrames / 2.0) / kStepSizeFrames - 3);
    for (std::size_t i = breakdownBegin; i < breakdownEnd; ++i) {
        track.onsets[i] = 0.01;
        for (int band = 0; band < mixxx::kQmOnsetBands; ++band) {
            track.bands[i * mixxx::kQmOnsetBands + band] = 0.01f;
        }
    }
    QVector<mixxx::audio::FramePos> trackerBeats = grid.mid(0, 64);
    trackerBeats += makeBeats(grid[64].value() / kSampleRate + 0.1, 112, 30);
    trackerBeats += grid.mid(96);

    const auto beatMap = beatMapOf(trackerBeats, track);
    EXPECT_LT(maxDistanceSecs(beatMap.beats, grid), 0.002);
}

TEST(QmBeatMapTest, KeepsConstantGridPhaseWhenHalfBeatIsAmbiguous) {
    constexpr int kBeatCount = 200;
    const auto beats = makeBeats(1.0, 120, kBeatCount);
    const KickHiHatTrack track = makeKickHiHatTrack(beats, true);
    EXPECT_LT(std::fabs(offBeatScore(beats, track)),
            0.5);
    // The grid is on the off-beats of the tracker, the half beat model can not
    // tell which is right: the grid keeps its phase.
    const mixxx::ConstantBeatGrid offBeatGrid{
            mixxx::Bpm(120), mixxx::audio::FramePos(0.25 * kSampleRate)};

    const auto beatMap = beatMapOf(beats, track, offBeatGrid);
    ASSERT_TRUE(beatMap.constantGrid);
    EXPECT_EQ(offBeatGrid.firstBeat, beatMap.constantGrid->firstBeat);
}

TEST(QmBeatMapTest, FollowsKicksWhenTrackerSwitchesToHiHats) {
    // The tracker follows the kicks, then the hi-hats.
    constexpr int kBeatCount = 200;
    const auto beats = makeBeats(1.0, 120, kBeatCount);
    const KickHiHatTrack track = makeKickHiHatTrack(beats);
    QVector<mixxx::audio::FramePos> trackerBeats = beats.mid(0, 100);
    trackerBeats += makeBeats(1.0 + 100 * 0.5 + 0.25, 120, kBeatCount - 101);

    const auto beatMap = beatMapOf(trackerBeats, track);
    EXPECT_LT(maxDistanceSecs(beatMap.beats, beats), 0.002);
}

TEST(QmBeatMapTest, FollowsOnsetsWhenTrackerTempoWobbles) {
    // The tracker is slightly too slow in the middle of a 120 BPM track and
    // catches up: the regions are joined into one at the tempo of the onsets.
    constexpr int kBeatCount = 240;
    const auto beats = makeBeats(1.0, 120, kBeatCount);
    const KickHiHatTrack track = makeKickHiHatTrack(beats);
    QVector<mixxx::audio::FramePos> trackerBeats = beats;
    for (int i = 80; i < 160; ++i) {
        // Up to 20 ms late.
        const double late = 0.020 * std::sin(M_PI * (i - 80) / 80.0);
        trackerBeats[i] += late * kSampleRate;
    }

    const auto beatMap = beatMapOf(trackerBeats, track);
    EXPECT_LT(maxDistanceSecs(beatMap.beats, beats), 0.002);
}

TEST(QmBeatMapTest, EmptyInput) {
    const auto beatMap =
            mixxx::makeQmBeatMap({}, {}, {}, kSampleRate, kStepSizeFrames, std::nullopt);
    EXPECT_TRUE(beatMap.beats.isEmpty());
    EXPECT_FALSE(beatMap.constantGrid);
}

TEST(QmBeatMapTest, FewBeats) {
    for (int count = 1; count <= 3; ++count) {
        const auto beats = makeBeats(1.0, 120, count);
        const KickHiHatTrack track = makeKickHiHatTrack(beats);
        const auto beatMap = beatMapOf(beats, track);
        if (count == 1) {
            EXPECT_TRUE(beatMap.beats.isEmpty());
        } else {
            EXPECT_LT(maxDistanceSecs(beatMap.beats, beats), 0.002);
        }
        EXPECT_FALSE(beatMap.constantGrid);
    }
}

// A region from firstBeatSecs at bpm; as the last region of a list, the end.
mixxx::qmbeatmap::Region makeRegion(double firstBeatSecs, double bpm = 120) {
    return {firstBeatSecs * kSampleRate, 60 * kSampleRate / bpm};
}

// The beats of regions, including the end marker.
QVector<mixxx::audio::FramePos> regionBeats(const mixxx::qmbeatmap::Regions& regions) {
    QVector<mixxx::audio::FramePos> beats;
    for (std::size_t i = 0; i + 1 < regions.size(); ++i) {
        const auto count = std::lround(
                (regions[i + 1].firstBeat - regions[i].firstBeat) / regions[i].beatLength);
        for (long k = 0; k < count; ++k) {
            beats.append(mixxx::audio::FramePos(
                    regions[i].firstBeat + k * regions[i].beatLength));
        }
    }
    beats.append(mixxx::audio::FramePos(regions.back().firstBeat));
    return beats;
}

mixxx::qmbeatmap::Regions mergeNeighbours(const mixxx::qmbeatmap::Regions& regions) {
    const KickHiHatTrack track = makeKickHiHatTrack(regionBeats(regions));
    return mixxx::qmbeatmap::mergeNeighbours(
            regions, track.onsets, track.bands, kSampleRate, kStepSizeFrames);
}

mixxx::qmbeatmap::Regions bridge(const mixxx::qmbeatmap::Regions& regions) {
    const KickHiHatTrack track = makeKickHiHatTrack(regionBeats(regions));
    return mixxx::qmbeatmap::bridge(
            regions, track.onsets, track.bands, kSampleRate, kStepSizeFrames);
}

TEST(QmBeatMapTest, MergesNeighboursWithNearlyTheSameTempo) {
    // 64 beats at 120 BPM, then 64 beats 0.2 % faster.
    const auto merged = mergeNeighbours(
            {makeRegion(0.5), makeRegion(32.5, 120.24), makeRegion(32.5 + 64 * 60 / 120.24)});
    ASSERT_EQ(2, merged.size());
    EXPECT_NEAR(0.5 * kSampleRate, merged[0].firstBeat, 0.03 * kSampleRate);
    EXPECT_NEAR(60 * kSampleRate / 120.12, merged[0].beatLength, 0.002 * kSampleRate);
}

TEST(QmBeatMapTest, KeepsNeighboursApartAcrossPhaseJump) {
    const auto regions = mergeNeighbours(
            {makeRegion(0.5), makeRegion(32.6), makeRegion(64.6)});
    EXPECT_EQ(3, regions.size());
}

TEST(QmBeatMapTest, KeepsNeighboursApartAcrossTempoChange) {
    // 1 % faster, more than mergeNeighbours() joins.
    const auto regions = mergeNeighbours(
            {makeRegion(0.5), makeRegion(32.5, 121.2), makeRegion(32.5 + 64 * 60 / 121.2)});
    EXPECT_EQ(3, regions.size());
}

TEST(QmBeatMapTest, BridgesShortRegionInPhase) {
    // 64 beats at 120 BPM, 7 beats at 105 BPM, then 64 beats at 120 BPM on
    // the grid of the first region.
    const auto bridged = bridge({makeRegion(0.5),
            makeRegion(32.5, 105),
            makeRegion(36.5),
            makeRegion(68.5)});
    ASSERT_EQ(2, bridged.size());
    EXPECT_NEAR(0.5 * kSampleRate, bridged[0].firstBeat, 0.01 * kSampleRate);
    EXPECT_NEAR(0.5 * kSampleRate, bridged[0].beatLength, 0.0005 * kSampleRate);
}

TEST(QmBeatMapTest, KeepsRegionsApartOutOfPhase) {
    // The last region is 100 ms off the grid of the first.
    const auto regions = bridge(
            {makeRegion(0.5), makeRegion(32.5, 102.4), makeRegion(36.6), makeRegion(68.6)});
    EXPECT_EQ(4, regions.size());
}

TEST(QmBeatMapTest, KeepsRegionsApartAcrossLongGap) {
    // 35 beats between the regions, more than bridge() joins across.
    const auto regions = bridge(
            {makeRegion(0.5), makeRegion(32.5, 105), makeRegion(52.5), makeRegion(84.5)});
    EXPECT_EQ(4, regions.size());
}

std::vector<double> makePositions(double firstSecs, int count) {
    std::vector<double> positions;
    for (int i = 0; i < count; ++i) {
        positions.push_back((firstSecs + i * 0.5) * kSampleRate);
    }
    return positions;
}

// The positions of 40 beats at 120 BPM, then 40 beats starting gapSecs after
// the last one.
std::vector<double> makePositionsWithGap(double gapSecs) {
    std::vector<double> positions = makePositions(0.5, 40);
    const std::vector<double> after = makePositions(20 + gapSecs, 40);
    positions.insert(positions.end(), after.begin(), after.end());
    return positions;
}

void expectEvenlySpaced(const std::vector<double>& positions) {
    for (std::size_t i = 1; i < positions.size(); ++i) {
        const double interval = (positions[i] - positions[i - 1]) / kSampleRate;
        EXPECT_GE(interval, 0.75 * 0.5) << "interval " << i;
        EXPECT_LE(interval, 1.25 * 0.5) << "interval " << i;
    }
}

TEST(QmBeatMapTest, SmoothBoundariesKeepsEvenBeats) {
    const std::vector<double> positions = makePositions(0.5, 80);
    EXPECT_EQ(positions, mixxx::qmbeatmap::smoothBoundaries(positions));
}

TEST(QmBeatMapTest, SmoothBoundariesSpreadsLongAndShortIntervals) {
    for (const double gapSecs : {0.75, 0.25}) {
        SCOPED_TRACE(gapSecs);
        const std::vector<double> positions = makePositionsWithGap(gapSecs);
        const std::vector<double> smoothed = mixxx::qmbeatmap::smoothBoundaries(positions);
        expectEvenlySpaced(smoothed);
        // Only the beats next to the gap move.
        ASSERT_GE(smoothed.size(), 70);
        EXPECT_TRUE(std::equal(positions.begin(), positions.begin() + 30, smoothed.begin()));
        EXPECT_TRUE(std::equal(positions.end() - 30, positions.end(), smoothed.end() - 30));
    }
}

} // namespace
