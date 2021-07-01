#include <gtest/gtest.h>

#include <QDebug>

#include "audio/frame.h"

class FrameTest : public testing::Test {
};

TEST_F(FrameTest, TestFramePosValid) {
    EXPECT_TRUE(mixxx::audio::FramePos(100).isValid());
    EXPECT_TRUE(mixxx::audio::FramePos(2000).isValid());
    EXPECT_TRUE(mixxx::audio::FramePos(0).isValid());
    EXPECT_TRUE(mixxx::audio::FramePos(-1).isValid());
    EXPECT_TRUE(mixxx::audio::FramePos(99999).isValid());
    EXPECT_TRUE(mixxx::audio::FramePos(-99999).isValid());
    EXPECT_TRUE(mixxx::audio::FramePos(128.5).isValid());
    EXPECT_TRUE(mixxx::audio::FramePos(135.67).isValid());
    EXPECT_FALSE(mixxx::audio::FramePos().isValid());
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
