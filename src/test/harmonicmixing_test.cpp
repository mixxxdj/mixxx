#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "library/autodj/harmonicmixing.h"

using ::testing::UnorderedElementsAre;

namespace {

using mixxx::HarmonicMixing;
using mixxx::track::io::key::ChromaticKey;

class HarmonicMixingTest : public testing::Test {
};

TEST_F(HarmonicMixingTest, BpmRangeRequiresPositiveValues) {
    EXPECT_FALSE(HarmonicMixing::bpmRange(0.0, 6.0).isValid());
    EXPECT_FALSE(HarmonicMixing::bpmRange(-128.0, 6.0).isValid());
    EXPECT_FALSE(HarmonicMixing::bpmRange(128.0, 0.0).isValid());
    EXPECT_FALSE(HarmonicMixing::bpmRange(128.0, -1.0).isValid());
}

TEST_F(HarmonicMixingTest, BpmRangeIsCenteredAroundTheReferenceTempo) {
    const auto range = HarmonicMixing::bpmRange(120.0, 6.0);
    ASSERT_TRUE(range.isValid());
    // 6% of 120 BPM is 7.2 BPM
    EXPECT_NEAR(112.8, range.min, 1e-9);
    EXPECT_NEAR(127.2, range.max, 1e-9);
}

TEST_F(HarmonicMixingTest, DocumentsTheDefaultTolerance) {
    const auto range = HarmonicMixing::bpmRange(
            120.0, HarmonicMixing::kDefaultBpmTolerancePercent);
    ASSERT_TRUE(range.isValid());
    EXPECT_NEAR(112.8, range.min, 1e-9);
    EXPECT_NEAR(127.2, range.max, 1e-9);
}

TEST_F(HarmonicMixingTest, BpmFallbackRangesStartWithTheConfiguredTolerance) {
    const auto ranges = HarmonicMixing::bpmFallbackRanges(120.0, 6.0);

    // The window the user configured, then wider ones and finally "ignore the
    // tempo", so the caller never gives up on mixing by key.
    ASSERT_GE(ranges.size(), 2U);
    EXPECT_NEAR(112.8, ranges.first().min, 1e-9);
    EXPECT_NEAR(127.2, ranges.first().max, 1e-9);
    EXPECT_FALSE(ranges.constLast().isValid());
}

TEST_F(HarmonicMixingTest, BpmFallbackRangesWidenTheTolerance) {
    const auto ranges = HarmonicMixing::bpmFallbackRanges(120.0, 6.0);

    ASSERT_TRUE(ranges.constFirst().isValid());
    for (auto range = ranges.cbegin() + 1; range != ranges.cend(); ++range) {
        if (!range->isValid()) {
            continue;
        }
        // Every window either contains the previous one or is identical to it.
        EXPECT_LE(range->min, ranges.constFirst().min);
        EXPECT_GE(range->max, ranges.constFirst().max);
    }
}

TEST_F(HarmonicMixingTest, BpmFallbackRangesRequireAReferenceTempo) {
    EXPECT_TRUE(HarmonicMixing::bpmFallbackRanges(0.0, 6.0).isEmpty());
    EXPECT_TRUE(HarmonicMixing::bpmFallbackRanges(120.0, 0.0).isEmpty());
}

TEST_F(HarmonicMixingTest, BpmFallbackRangesSkipTheWidenedWindowForHugeTolerances) {
    // 40% * 3 would reach below zero BPM, so the widened window cannot be used
    // and the fallback goes straight to "ignore the tempo".
    const auto ranges = HarmonicMixing::bpmFallbackRanges(120.0, 40.0);
    ASSERT_EQ(2U, ranges.size());
    EXPECT_TRUE(ranges.constFirst().isValid());
    EXPECT_FALSE(ranges.constLast().isValid());
}

TEST_F(HarmonicMixingTest, BpmCompatibility) {
    // 120 BPM with a 5% tolerance => 114 .. 126 BPM
    EXPECT_TRUE(HarmonicMixing::isBpmCompatible(120.0, 120.0, 5.0));
    EXPECT_TRUE(HarmonicMixing::isBpmCompatible(120.0, 114.0, 5.0));
    EXPECT_TRUE(HarmonicMixing::isBpmCompatible(120.0, 126.0, 5.0));
    EXPECT_FALSE(HarmonicMixing::isBpmCompatible(120.0, 113.9, 5.0));
    EXPECT_FALSE(HarmonicMixing::isBpmCompatible(120.0, 126.1, 5.0));
}

TEST_F(HarmonicMixingTest, UnknownTempoNeverMatches) {
    EXPECT_FALSE(HarmonicMixing::isBpmCompatible(0.0, 120.0, 5.0));
    EXPECT_FALSE(HarmonicMixing::isBpmCompatible(120.0, 0.0, 5.0));
}

TEST_F(HarmonicMixingTest, CompatibleKeysFollowTheCircleOfFifths) {
    EXPECT_THAT(HarmonicMixing::compatibleKeys(mixxx::track::io::key::C_MAJOR),
            UnorderedElementsAre(mixxx::track::io::key::C_MAJOR,
                    mixxx::track::io::key::A_MINOR,
                    mixxx::track::io::key::G_MAJOR,
                    mixxx::track::io::key::E_MINOR,
                    mixxx::track::io::key::F_MAJOR,
                    mixxx::track::io::key::D_MINOR));
}

TEST_F(HarmonicMixingTest, InvalidKeyHasNoCompatibleKeys) {
    EXPECT_TRUE(HarmonicMixing::compatibleKeys(mixxx::track::io::key::INVALID).isEmpty());
}

TEST_F(HarmonicMixingTest, KeyCompatibility) {
    EXPECT_TRUE(HarmonicMixing::isKeyCompatible(
            mixxx::track::io::key::C_MAJOR, mixxx::track::io::key::C_MAJOR));
    // Relative minor key.
    EXPECT_TRUE(HarmonicMixing::isKeyCompatible(
            mixxx::track::io::key::C_MAJOR, mixxx::track::io::key::A_MINOR));
    // Direct neighbours on the circle of fifths.
    EXPECT_TRUE(HarmonicMixing::isKeyCompatible(
            mixxx::track::io::key::C_MAJOR, mixxx::track::io::key::G_MAJOR));
    EXPECT_TRUE(HarmonicMixing::isKeyCompatible(
            mixxx::track::io::key::C_MAJOR, mixxx::track::io::key::F_MAJOR));
    // Two steps away on the circle of fifths.
    EXPECT_FALSE(HarmonicMixing::isKeyCompatible(
            mixxx::track::io::key::C_MAJOR, mixxx::track::io::key::D_MAJOR));
    EXPECT_FALSE(HarmonicMixing::isKeyCompatible(
            mixxx::track::io::key::C_MAJOR, mixxx::track::io::key::INVALID));
}

} // namespace
