#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <vector>

#include "library/autodj/smart/beatmatch.h"

TEST(BeatmatchTest, MatchesCloseTempos) {
    // 120 BPM track under a 124 BPM track plays 3.3% faster.
    const auto r = beatmatch::matchRatio(124.0, 120.0, 5.0);
    ASSERT_TRUE(r.has_value());
    EXPECT_NEAR(124.0 / 120.0, *r, 1e-9);
    EXPECT_NEAR(1.0, *beatmatch::matchRatio(128.0, 128.0, 5.0), 1e-9);
}

TEST(BeatmatchTest, RefusesTooBigAChange) {
    // The Promise (118.1) -> Rock The Casbah (129.5): about 9%.
    EXPECT_FALSE(beatmatch::matchRatio(118.1, 129.5, 5.0).has_value());
    EXPECT_FALSE(beatmatch::matchRatio(0.0, 120.0, 5.0).has_value());
    EXPECT_FALSE(beatmatch::matchRatio(120.0, 0.0, 5.0).has_value());
}

TEST(BeatmatchTest, UsesHalfAndDoubleTime) {
    // Cupid Shuffle (71.8) under Smalltown Boy at 144 BPM: plays at double
    // time, so the ratio is small, not ~2.
    const auto r = beatmatch::matchRatio(144.0, 71.8, 5.0);
    ASSERT_TRUE(r.has_value());
    EXPECT_NEAR(144.0 / (2.0 * 71.8), *r, 1e-9);
    const auto r2 = beatmatch::matchRatio(71.8, 144.0, 5.0);
    ASSERT_TRUE(r2.has_value());
    EXPECT_NEAR(2.0 * 71.8 / 144.0, *r2, 1e-9);
}

TEST(BeatmatchTest, GlidesBackToOwnTempo) {
    EXPECT_DOUBLE_EQ(1.04, beatmatch::glideRatio(1.04, 0.0, 30.0));
    EXPECT_NEAR(1.02, beatmatch::glideRatio(1.04, 15.0, 30.0), 1e-9);
    EXPECT_DOUBLE_EQ(1.0, beatmatch::glideRatio(1.04, 30.0, 30.0));
    EXPECT_DOUBLE_EQ(1.0, beatmatch::glideRatio(0.97, 99.0, 30.0));
    // Speed of change: at most 5% over 30 s = 0.17% per second.
    const double perSecond = std::abs(beatmatch::glideRatio(1.05, 1.0, 30.0) - 1.05);
    EXPECT_LT(perSecond, 0.002);
}

TEST(BeatmatchTest, BassSwapsHardAtTheMiddle) {
    EXPECT_TRUE(beatmatch::bassSwap(0.0).toLowKilled);
    EXPECT_FALSE(beatmatch::bassSwap(0.0).fromLowKilled);
    EXPECT_TRUE(beatmatch::bassSwap(0.49).toLowKilled);
    EXPECT_FALSE(beatmatch::bassSwap(0.5).toLowKilled);
    EXPECT_TRUE(beatmatch::bassSwap(0.5).fromLowKilled);
    EXPECT_TRUE(beatmatch::bassSwap(1.0).fromLowKilled);
    // Never both basses at once, never both cut.
    for (double p = 0.0; p <= 1.0; p += 0.05) {
        const auto s = beatmatch::bassSwap(p);
        EXPECT_NE(s.fromLowKilled, s.toLowKilled) << p;
    }
}

TEST(BeatmatchTest, StemMixNeverPlaysTwoVocalsTogether) {
    for (double p = 0.0; p <= 1.0001; p += 0.01) {
        const auto s = beatmatch::stemBlend(p);
        EXPECT_TRUE(s.fromVocals < 1e-9 || s.toVocals < 1e-9) << p;
        // Drums and bass swap hard, never both, never neither.
        EXPECT_NE(s.fromDrums > 0.5, s.toDrums > 0.5) << p;
        EXPECT_EQ(s.fromDrums, s.fromBass) << p;
        EXPECT_EQ(s.toDrums, s.toBass) << p;
    }
}

TEST(BeatmatchTest, StemMixStartsAndEndsClean) {
    const auto start = beatmatch::stemBlend(0.0);
    EXPECT_DOUBLE_EQ(start.fromVocals, 1.0);
    EXPECT_DOUBLE_EQ(start.fromInstrumental, 1.0);
    EXPECT_DOUBLE_EQ(start.fromDrums, 1.0);
    EXPECT_DOUBLE_EQ(start.toVocals, 0.0);
    EXPECT_DOUBLE_EQ(start.toInstrumental, 0.0);
    EXPECT_DOUBLE_EQ(start.toDrums, 0.0);
    const auto middle = beatmatch::stemBlend(0.5);
    EXPECT_NEAR(middle.fromVocals, 0.0, 1e-9); // old vocals gone by the swap
    EXPECT_DOUBLE_EQ(middle.toInstrumental, 1.0);
    EXPECT_DOUBLE_EQ(middle.toDrums, 1.0);
    EXPECT_DOUBLE_EQ(middle.fromDrums, 0.0);
    const auto end = beatmatch::stemBlend(1.0);
    EXPECT_NEAR(end.fromInstrumental, 0.0, 1e-9);
    EXPECT_NEAR(end.toVocals, 1.0, 1e-9);
    EXPECT_DOUBLE_EQ(end.toBass, 1.0);
}

TEST(BeatmatchTest, StemMixFadesSmoothly) {
    // No part jumps by more than a small step between nearby points,
    // except drums and bass at the swap.
    const double step = 0.01;
    for (double p = step; p <= 1.0001; p += step) {
        const auto a = beatmatch::stemBlend(p - step);
        const auto b = beatmatch::stemBlend(p);
        EXPECT_LT(std::fabs(a.fromVocals - b.fromVocals), 0.04) << p;
        EXPECT_LT(std::fabs(a.toVocals - b.toVocals), 0.04) << p;
        EXPECT_LT(std::fabs(a.fromInstrumental - b.fromInstrumental), 0.04) << p;
        EXPECT_LT(std::fabs(a.toInstrumental - b.toInstrumental), 0.04) << p;
    }
}

TEST(BeatmatchTest, EqBlendsMidsAndHighsGradually) {
    using beatmatch::eqBlend;
    using beatmatch::kEqBlendFloor;
    // Start: the outgoing track as the DJ set it, the incoming one held back.
    EXPECT_DOUBLE_EQ(1.0, eqBlend(0.0).fromMidHigh);
    EXPECT_DOUBLE_EQ(kEqBlendFloor, eqBlend(0.0).toMidHigh);
    // Middle: both at full.
    EXPECT_DOUBLE_EQ(1.0, eqBlend(0.5).fromMidHigh);
    EXPECT_DOUBLE_EQ(1.0, eqBlend(0.5).toMidHigh);
    // End: the outgoing track held back, the incoming one at full.
    EXPECT_DOUBLE_EQ(kEqBlendFloor, eqBlend(1.0).fromMidHigh);
    EXPECT_DOUBLE_EQ(1.0, eqBlend(1.0).toMidHigh);
    // Gradual, never a jump, never outside floor..1.
    double lastFrom = 1.0;
    double lastTo = 0.0;
    for (int i = 0; i <= 100; ++i) {
        const auto e = eqBlend(i / 100.0);
        EXPECT_LE(e.fromMidHigh, lastFrom);
        EXPECT_GE(e.toMidHigh, lastTo);
        EXPECT_LT(lastFrom - e.fromMidHigh, 0.02);
        EXPECT_LT(e.toMidHigh - lastTo, i == 0 ? 1.0 : 0.02);
        EXPECT_GE(e.fromMidHigh, kEqBlendFloor);
        EXPECT_LE(e.toMidHigh, 1.0);
        lastFrom = e.fromMidHigh;
        lastTo = e.toMidHigh;
    }
    // Out-of-range progress is clamped.
    EXPECT_DOUBLE_EQ(kEqBlendFloor, eqBlend(-1.0).toMidHigh);
    EXPECT_DOUBLE_EQ(kEqBlendFloor, eqBlend(2.0).fromMidHigh);
}

TEST(BeatmatchTest, BeatLockKeepsTheSameBeatLength) {
    // Outgoing beat 0.48 s long right now, incoming beat 0.5 s at its own
    // speed: play the incoming 0.5 / 0.48 faster. In line: no nudge.
    double slip = 1.0;
    EXPECT_NEAR(0.5 / 0.48, beatmatch::followRatio(100.25, 0.48, 7.25, 0.5, &slip), 1e-9);
    EXPECT_NEAR(0.0, slip, 1e-9);
    // A tiny slip is left alone.
    EXPECT_NEAR(0.5 / 0.48, beatmatch::followRatio(100.255, 0.48, 7.25, 0.5), 1e-9);
}

TEST(BeatmatchTest, BeatLockPullsASlippedBeatBack) {
    // The incoming beat is 0.1 beat late: faster (5 %), back in line in
    // about 2 beats.
    double slip = 0.0;
    const double faster = beatmatch::followRatio(100.1, 0.5, 7.0, 0.5, &slip);
    EXPECT_NEAR(0.1, slip, 1e-9);
    EXPECT_NEAR(1.05, faster, 1e-9);
    // A big slip (0.3 beat late): capped at 6 %.
    EXPECT_NEAR(1.0 + beatmatch::kMaxLockNudge,
            beatmatch::followRatio(100.3, 0.5, 7.0, 0.5),
            1e-9);
    // 0.04 beat early: 2 % slower.
    EXPECT_NEAR(0.98, beatmatch::followRatio(100.96, 0.5, 8.0, 0.5), 1e-9);
    // Beat numbers far apart do not matter, only where in the beat.
    EXPECT_NEAR(0.98, beatmatch::followRatio(5.96, 0.5, 300.0, 0.5), 1e-9);
    // Unknown beat lengths: leave the speed alone.
    EXPECT_DOUBLE_EQ(1.0, beatmatch::followRatio(1.0, 0.0, 1.0, 0.5));
}

TEST(BeatmatchTest, BeatLockFollowsADrummerWhoSpeedsUp) {
    // Simulate 60 s of a mix, in 20 ms steps (how often Auto DJ updates).
    // Outgoing: a live drummer, 120 BPM speeding up to about 126 BPM.
    // Incoming: steady 122 BPM. Starts in line.
    const auto fromBeatLen = [](double t) { return 0.5 / (1.0 + 0.0008 * t); };
    double fromBeat = 0.0;
    double toBeat = 0.0;
    const double toLen = 60.0 / 122.0;
    double worst = 0.0;
    for (double t = 0.0; t < 60.0; t += 0.02) {
        // The beat length Auto DJ measures is an average over a few beats,
        // so it lags about 2 s behind.
        const double measured = fromBeatLen(std::max(0.0, t - 2.0));
        const double ratio = beatmatch::followRatio(fromBeat, measured, toBeat, toLen);
        fromBeat += 0.02 / fromBeatLen(t);
        toBeat += 0.02 * ratio / toLen;
        double slip = fromBeat - toBeat;
        slip -= std::round(slip);
        worst = std::max(worst, std::fabs(slip));
    }
    // Never more than 2% of a beat apart (10 ms): nobody hears that.
    EXPECT_LT(worst, 0.02);
}

TEST(BeatmatchTest, StemBlendCanFadeTheDrums) {
    // The drums cross over gradually; the bass still swaps in the middle.
    double lastTo = -1.0;
    for (double p = 0.0; p <= 1.0; p += 0.05) {
        const beatmatch::StemBlend s = beatmatch::stemBlend(p, true);
        EXPECT_GE(s.toDrums, lastTo);
        lastTo = s.toDrums;
        EXPECT_EQ(p < 0.5 ? 0.0 : 1.0, s.toBass);
        EXPECT_EQ(p < 0.5 ? 1.0 : 0.0, s.fromBass);
    }
    EXPECT_DOUBLE_EQ(0.0, beatmatch::stemBlend(0.0, true).toDrums);
    EXPECT_NEAR(0.707, beatmatch::stemBlend(0.25, true).toDrums, 0.001);
    EXPECT_DOUBLE_EQ(1.0, beatmatch::stemBlend(0.25, true).fromDrums);
    EXPECT_NEAR(0.707, beatmatch::stemBlend(0.75, true).fromDrums, 0.001);
    EXPECT_NEAR(0.0, beatmatch::stemBlend(1.0, true).fromDrums, 1e-9);
    // Without it: the old hard swap.
    EXPECT_DOUBLE_EQ(0.0, beatmatch::stemBlend(0.25).toDrums);
    // With singing detection too.
    const beatmatch::VocalPlan plan;
    EXPECT_NEAR(0.707, beatmatch::stemBlend(0.25, plan, true).toDrums, 0.001);
}

namespace {
/// A drum part: a kick `kickAfter` seconds after every beat line (0.5 s
/// apart), 441 level values per second, like the waveform.
std::vector<float> drumLevel(double kickAfter, std::vector<double>* pBeats) {
    constexpr double kRate = 441.0;
    std::vector<float> level(static_cast<int>(40.0 * kRate), 0.03f);
    for (double t = 1.0; t < 38.0; t += 0.5) {
        pBeats->push_back(t);
        const double start = t + kickAfter;
        for (int i = static_cast<int>(start * kRate); i < static_cast<int>(level.size()); ++i) {
            const double since = i / kRate - start;
            if (since > 0.2) {
                break;
            }
            level[i] = std::max(level[i], static_cast<float>(0.9 * std::exp(-since / 0.05)));
        }
    }
    return level;
}
} // namespace

TEST(BeatmatchTest, KickOffsetFindsWhereTheKickStarts) {
    // Two songs, kicks 8 ms and 41 ms after their lines (Pretty Boys and
    // Tell Me Why): measured within the waveform's resolution (2.3 ms).
    std::vector<double> beatsA;
    const auto a = beatmatch::kickOffset(drumLevel(0.008, &beatsA), 441.0, beatsA);
    std::vector<double> beatsB;
    const auto b = beatmatch::kickOffset(drumLevel(0.041, &beatsB), 441.0, beatsB);
    ASSERT_TRUE(a.has_value());
    ASSERT_TRUE(b.has_value());
    EXPECT_NEAR(0.008, *a, 0.003);
    EXPECT_NEAR(0.041, *b, 0.003);
    EXPECT_NEAR(0.033, *b - *a, 0.003);
    // A kick just before the line.
    std::vector<double> beatsC;
    const auto c = beatmatch::kickOffset(drumLevel(-0.015, &beatsC), 441.0, beatsC);
    ASSERT_TRUE(c.has_value());
    EXPECT_NEAR(-0.015, *c, 0.003);
}

TEST(BeatmatchTest, KickOffsetNeedsDrums) {
    // No drums (a quiet, flat level): no answer.
    std::vector<double> beats;
    for (double t = 1.0; t < 38.0; t += 0.5) {
        beats.push_back(t);
    }
    EXPECT_FALSE(beatmatch::kickOffset(std::vector<float>(17640, 0.01f), 441.0, beats));
    // Too few beats.
    std::vector<double> few(beats.begin(), beats.begin() + 10);
    std::vector<double> ignored;
    EXPECT_FALSE(beatmatch::kickOffset(drumLevel(0.01, &ignored), 441.0, few));
}
