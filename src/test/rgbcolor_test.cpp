#include "util/color/rgbcolor.h"

#include <gtest/gtest.h>

#include "test/mixxxtest.h"

namespace mixxx {

TEST(RgbColorTest, DefaultIsInvalid) {
    EXPECT_FALSE(RgbColor().isValid());
}

TEST(RgbColorTest, RgbColorCodeCtor) {
    EXPECT_TRUE(RgbColor(0x000000).isValid());
    EXPECT_EQ(0x000000, RgbColor(0x000000).code());
    EXPECT_TRUE(RgbColor(0xFF0000).isValid());
    EXPECT_EQ(0xFF0000, RgbColor(0xFF0000).code());
    EXPECT_TRUE(RgbColor(0x00FF00).isValid());
    EXPECT_EQ(0x00FF00, RgbColor(0x00FF00).code());
    EXPECT_TRUE(RgbColor(0x0000FF).isValid());
    EXPECT_EQ(0x0000FF, RgbColor(0x0000FF).code());
    EXPECT_TRUE(RgbColor(0xFFFFFF).isValid());
    EXPECT_EQ(0xFFFFFF, RgbColor(0xFFFFFF).code());
}

TEST(RgbColorTest, QColorCtor) {
    EXPECT_TRUE(RgbColor(QColor::fromRgb(0x000000)).isValid());
    EXPECT_EQ(0x000000, RgbColor(QColor::fromRgb(0x000000)).code());
    EXPECT_TRUE(RgbColor(QColor::fromRgb(0xFF0000)).isValid());
    EXPECT_EQ(0xFF0000, RgbColor(QColor::fromRgb(0xFF0000)).code());
    EXPECT_TRUE(RgbColor(QColor::fromRgb(0x00FF00)).isValid());
    EXPECT_EQ(0x00FF00, RgbColor(QColor::fromRgb(0x00FF00)).code());
    EXPECT_TRUE(RgbColor(QColor::fromRgb(0x0000FF)).isValid());
    EXPECT_EQ(0x0000FF, RgbColor(QColor::fromRgb(0x0000FF)).code());
    EXPECT_TRUE(RgbColor(QColor::fromRgb(0xFFFFFF)).isValid());
    EXPECT_EQ(0xFFFFFF, RgbColor(QColor::fromRgb(0xFFFFFF)).code());
}

TEST(RgbColorTest, FromRgbColorCodeWithAlpha) {
    EXPECT_TRUE(RgbColor::fromCode(0xF0000000).isValid());
    EXPECT_EQ(0x000000, RgbColor::fromCode(0xF0000000).code());
    EXPECT_TRUE(RgbColor::fromCode(0xF0FF0000).isValid());
    EXPECT_EQ(0xFF0000, RgbColor::fromCode(0xF0FF0000).code());
    EXPECT_TRUE(RgbColor::fromCode(0xF000FF00).isValid());
    EXPECT_EQ(0x00FF00, RgbColor::fromCode(0xF000FF00).code());
    EXPECT_TRUE(RgbColor::fromCode(0xF00000FF).isValid());
    EXPECT_EQ(0x0000FF, RgbColor::fromCode(0xF00000FF).code());
    EXPECT_TRUE(RgbColor::fromCode(0xF0FFFFFF).isValid());
    EXPECT_EQ(0xFFFFFF, RgbColor::fromCode(0xF0FFFFFF).code());
}

TEST(RgbColorTest, FromQColorWithAlpha) {
    EXPECT_TRUE(RgbColor::fromCode(0xF0000000).isValid());
    EXPECT_EQ(0x000000, RgbColor::fromCode(0xF0000000).code());
    EXPECT_TRUE(RgbColor::fromCode(0xF0FF0000).isValid());
    EXPECT_EQ(0xFF0000, RgbColor::fromCode(0xF0FF0000).code());
    EXPECT_TRUE(RgbColor::fromCode(0xF000FF00).isValid());
    EXPECT_EQ(0x00FF00, RgbColor::fromCode(0xF000FF00).code());
    EXPECT_TRUE(RgbColor::fromCode(0xF00000FF).isValid());
    EXPECT_EQ(0x0000FF, RgbColor::fromCode(0xF00000FF).code());
    EXPECT_TRUE(RgbColor::fromCode(0xF0FFFFFF).isValid());
    EXPECT_EQ(0xFFFFFF, RgbColor::fromCode(0xF0FFFFFF).code());
}

} // namespace mixxx
