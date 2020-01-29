#include "util/color/rgbcolor.h"

#include <gtest/gtest.h>

#include "test/mixxxtest.h"

namespace mixxx {

TEST(RgbColorTest, DefaultIsInvalid) {
    EXPECT_FALSE(RgbColor().isValid());
    EXPECT_FALSE(RgbColor().optionalCode());
}

TEST(RgbColorTest, RgbColorCodeCtor) {
    EXPECT_TRUE(RgbColor(0x000000).isValid());
    EXPECT_EQ(0x000000, *RgbColor(0x000000).optionalCode());
    EXPECT_TRUE(RgbColor(0xFF0000).isValid());
    EXPECT_EQ(0xFF0000, *RgbColor(0xFF0000).optionalCode());
    EXPECT_TRUE(RgbColor(0x00FF00).isValid());
    EXPECT_EQ(0x00FF00, *RgbColor(0x00FF00).optionalCode());
    EXPECT_TRUE(RgbColor(0x0000FF).isValid());
    EXPECT_EQ(0x0000FF, *RgbColor(0x0000FF).optionalCode());
    EXPECT_TRUE(RgbColor(0xFFFFFF).isValid());
    EXPECT_EQ(0xFFFFFF, *RgbColor(0xFFFFFF).optionalCode());
}

TEST(RgbColorTest, QColorCtor) {
    EXPECT_TRUE(RgbColor(QColor::fromRgb(0x000000)).isValid());
    EXPECT_EQ(0x000000, *RgbColor(QColor::fromRgb(0x000000)).optionalCode());
    EXPECT_TRUE(RgbColor(QColor::fromRgb(0xFF0000)).isValid());
    EXPECT_EQ(0xFF0000, *RgbColor(QColor::fromRgb(0xFF0000)).optionalCode());
    EXPECT_TRUE(RgbColor(QColor::fromRgb(0x00FF00)).isValid());
    EXPECT_EQ(0x00FF00, *RgbColor(QColor::fromRgb(0x00FF00)).optionalCode());
    EXPECT_TRUE(RgbColor(QColor::fromRgb(0x0000FF)).isValid());
    EXPECT_EQ(0x0000FF, *RgbColor(QColor::fromRgb(0x0000FF)).optionalCode());
    EXPECT_TRUE(RgbColor(QColor::fromRgb(0xFFFFFF)).isValid());
    EXPECT_EQ(0xFFFFFF, *RgbColor(QColor::fromRgb(0xFFFFFF)).optionalCode());
}

TEST(RgbColorTest, FromRgbColorCodeWithAlpha) {
    EXPECT_TRUE(RgbColor(0xF0000000).isValid());
    EXPECT_EQ(0x000000, *RgbColor(0xF0000000).optionalCode());
    EXPECT_TRUE(RgbColor(0xF0FF0000).isValid());
    EXPECT_EQ(0xFF0000, *RgbColor(0xF0FF0000).optionalCode());
    EXPECT_TRUE(RgbColor(0xF000FF00).isValid());
    EXPECT_EQ(0x00FF00, *RgbColor(0xF000FF00).optionalCode());
    EXPECT_TRUE(RgbColor(0xF00000FF).isValid());
    EXPECT_EQ(0x0000FF, *RgbColor(0xF00000FF).optionalCode());
    EXPECT_TRUE(RgbColor(0xF0FFFFFF).isValid());
    EXPECT_EQ(0xFFFFFF, *RgbColor(0xF0FFFFFF).optionalCode());
}

TEST(RgbColorTest, FromQColorWithAlpha) {
    EXPECT_TRUE(RgbColor(0xF0000000).isValid());
    EXPECT_EQ(0x000000, *RgbColor(0xF0000000).optionalCode());
    EXPECT_TRUE(RgbColor(0xF0FF0000).isValid());
    EXPECT_EQ(0xFF0000, *RgbColor(0xF0FF0000).optionalCode());
    EXPECT_TRUE(RgbColor(0xF000FF00).isValid());
    EXPECT_EQ(0x00FF00, *RgbColor(0xF000FF00).optionalCode());
    EXPECT_TRUE(RgbColor(0xF00000FF).isValid());
    EXPECT_EQ(0x0000FF, *RgbColor(0xF00000FF).optionalCode());
    EXPECT_TRUE(RgbColor(0xF0FFFFFF).isValid());
    EXPECT_EQ(0xFFFFFF, *RgbColor(0xF0FFFFFF).optionalCode());
}

} // namespace mixxx
