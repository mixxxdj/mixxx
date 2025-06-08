#include <gtest/gtest.h>

#include <QDebug>

#include "audio/frame.h"
#include "util/fpclassify.h"

class FrameTest : public testing::Test {
};

TEST_F(FrameTest, TestFramePosValid) {
    EXPECT_TRUE(mixxx::audio::FramePos(100).isValid());
    EXPECT_TRUE(mixxx::audio::FramePos(2000).isValid());
    EXPECT_TRUE(mixxx::audio::FramePos(-1).isValid());
    EXPECT_TRUE(mixxx::audio::FramePos(99999).isValid());
    EXPECT_TRUE(mixxx::audio::FramePos(-99999).isValid());
    EXPECT_TRUE(mixxx::audio::FramePos(128.5).isValid());
    EXPECT_TRUE(mixxx::audio::FramePos(135.67).isValid());
    // Denormals
    EXPECT_TRUE(mixxx::audio::FramePos(0.0).isValid());
    EXPECT_TRUE(mixxx::audio::FramePos(std::numeric_limits<double>::min() / 2.0).isValid());
    EXPECT_FALSE(mixxx::audio::FramePos(util_double_infinity()).isValid());
    // NaN
    EXPECT_FALSE(mixxx::audio::FramePos().isValid());
    EXPECT_FALSE(mixxx::audio::FramePos(std::numeric_limits<double>::quiet_NaN()).isValid());
    EXPECT_FALSE(mixxx::audio::FramePos(std::numeric_limits<double>::signaling_NaN()).isValid());
}

TEST_F(FrameTest, TestFramePosFractional) {
    EXPECT_FALSE(mixxx::audio::FramePos(100).isFractional());
    EXPECT_FALSE(mixxx::audio::FramePos(2000).isFractional());
    EXPECT_FALSE(mixxx::audio::FramePos(0).isFractional());
    EXPECT_FALSE(mixxx::audio::FramePos(-1).isFractional());
    EXPECT_FALSE(mixxx::audio::FramePos(99999).isFractional());
    EXPECT_FALSE(mixxx::audio::FramePos(-99999).isFractional());
    EXPECT_TRUE(mixxx::audio::FramePos(128.5).isFractional());
    EXPECT_TRUE(mixxx::audio::FramePos(135.67).isFractional());
}

TEST_F(FrameTest, TestFramePosEquality) {
    EXPECT_EQ(mixxx::audio::FramePos(0), mixxx::audio::FramePos(0));
    EXPECT_EQ(mixxx::audio::FramePos(-100), mixxx::audio::FramePos(-100));
    EXPECT_EQ(mixxx::audio::FramePos(100), mixxx::audio::FramePos(100));
    EXPECT_EQ(mixxx::audio::FramePos(50) * 2, mixxx::audio::FramePos(100));

    EXPECT_NE(mixxx::audio::FramePos(100), mixxx::audio::FramePos(200));
    EXPECT_NE(mixxx::audio::FramePos(100), mixxx::audio::FramePos());
    EXPECT_NE(mixxx::audio::FramePos(0), mixxx::audio::FramePos());

    // Check that invalid positions are equal to each other
    EXPECT_EQ(mixxx::audio::FramePos(), mixxx::audio::kInvalidFramePos);
    EXPECT_EQ(mixxx::audio::FramePos(),
            mixxx::audio::FramePos(mixxx::audio::FramePos::kInvalidValue));
    EXPECT_EQ(mixxx::audio::FramePos(),
            mixxx::audio::FramePos(std::numeric_limits<
                    mixxx::audio::FramePos::value_t>::quiet_NaN()));
    EXPECT_EQ(mixxx::audio::FramePos(),
            mixxx::audio::FramePos(util_double_infinity()));
    EXPECT_EQ(mixxx::audio::FramePos(),
            mixxx::audio::FramePos(-util_double_infinity()));
    EXPECT_EQ(mixxx::audio::FramePos(std::numeric_limits<
                      mixxx::audio::FramePos::value_t>::quiet_NaN()),
            mixxx::audio::FramePos(util_double_infinity()));
}

TEST_F(FrameTest, LowerFrameBoundary) {
    EXPECT_EQ(mixxx::audio::FramePos(0), mixxx::audio::FramePos(0).toLowerFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(0), mixxx::audio::FramePos(0.5).toLowerFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(0), mixxx::audio::FramePos(0.9).toLowerFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(100), mixxx::audio::FramePos(100).toLowerFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(100), mixxx::audio::FramePos(100.1).toLowerFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(100), mixxx::audio::FramePos(100.9).toLowerFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(-100), mixxx::audio::FramePos(-99.9).toLowerFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(-100), mixxx::audio::FramePos(-99.1).toLowerFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(-101), mixxx::audio::FramePos(-100.1).toLowerFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(-101), mixxx::audio::FramePos(-100.9).toLowerFrameBoundary());
}

TEST_F(FrameTest, UpperFrameBoundary) {
    EXPECT_EQ(mixxx::audio::FramePos(0), mixxx::audio::FramePos(0).toUpperFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(1), mixxx::audio::FramePos(0.5).toUpperFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(1), mixxx::audio::FramePos(0.9).toUpperFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(100), mixxx::audio::FramePos(100).toUpperFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(101), mixxx::audio::FramePos(100.1).toUpperFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(101), mixxx::audio::FramePos(100.9).toUpperFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(-99), mixxx::audio::FramePos(-99.9).toUpperFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(-99), mixxx::audio::FramePos(-99.1).toUpperFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(-100), mixxx::audio::FramePos(-100.1).toUpperFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(-100), mixxx::audio::FramePos(-100.9).toUpperFrameBoundary());
}

TEST_F(FrameTest, NearestFrameBoundary) {
    EXPECT_EQ(mixxx::audio::FramePos(0), mixxx::audio::FramePos(0).toNearestFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(1), mixxx::audio::FramePos(0.5).toNearestFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(1), mixxx::audio::FramePos(0.9).toNearestFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(100), mixxx::audio::FramePos(100).toNearestFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(100), mixxx::audio::FramePos(100.1).toNearestFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(101), mixxx::audio::FramePos(100.9).toNearestFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(-100), mixxx::audio::FramePos(-99.9).toNearestFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(-99), mixxx::audio::FramePos(-99.1).toNearestFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(-100),
            mixxx::audio::FramePos(-100.1).toNearestFrameBoundary());
    EXPECT_EQ(mixxx::audio::FramePos(-101),
            mixxx::audio::FramePos(-100.9).toNearestFrameBoundary());
}
