#include <gtest/gtest.h>

#include <QDebug>

#include "audio/frame.h"

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
    EXPECT_FALSE(mixxx::audio::FramePos(std::numeric_limits<double>::infinity()).isValid());
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
            mixxx::audio::FramePos(std::numeric_limits<
                    mixxx::audio::FramePos::value_t>::infinity()));
    EXPECT_EQ(mixxx::audio::FramePos(),
            mixxx::audio::FramePos(
                    -std::numeric_limits<
                            mixxx::audio::FramePos::value_t>::infinity()));
    EXPECT_EQ(mixxx::audio::FramePos(std::numeric_limits<
                      mixxx::audio::FramePos::value_t>::quiet_NaN()),
            mixxx::audio::FramePos(std::numeric_limits<
                    mixxx::audio::FramePos::value_t>::infinity()));
}
