#include "util/color/rgbcolor.h"

#include <gtest/gtest.h>

#include "test/mixxxtest.h"

namespace mixxx {

TEST(RgbColorTest, DefaultIsUndefined) {
    EXPECT_FALSE(RgbColor());
    EXPECT_FALSE(static_cast<bool>(RgbColor().optional()));
}

TEST(RgbColorTest, FromRgbColorCode) {
    EXPECT_TRUE(RgbColor(0x000000));
    EXPECT_EQ(0x000000, *RgbColor(0x000000).optional());
    EXPECT_TRUE(RgbColor(0xFF0000));
    EXPECT_EQ(0xFF0000, *RgbColor(0xFF0000).optional());
    EXPECT_TRUE(RgbColor(0x00FF00));
    EXPECT_EQ(0x00FF00, *RgbColor(0x00FF00).optional());
    EXPECT_TRUE(RgbColor(0x0000FF));
    EXPECT_EQ(0x0000FF, *RgbColor(0x0000FF).optional());
    EXPECT_TRUE(RgbColor(0xFFFFFF));
    EXPECT_EQ(0xFFFFFF, *RgbColor(0xFFFFFF).optional());
}

TEST(RgbColorTest, FromOptionalRgbColorCode) {
    EXPECT_EQ(RgbColor(), RgbColor(std::nullopt));
    EXPECT_EQ(RgbColor(0x123456), RgbColor(std::make_optional(RgbColorCode(0x123456))));
}

TEST(RgbColorTest, FromQColor) {
    EXPECT_FALSE(RgbColor(QColor()));
    EXPECT_EQ(RgbColor(), RgbColor(QColor()));
    EXPECT_TRUE(RgbColor(QColor::fromRgb(0x000000)));
    EXPECT_EQ(RgbColor(0x000000), RgbColor(QColor::fromRgb(0x000000)));
    EXPECT_TRUE(RgbColor(QColor::fromRgb(0xFF0000)));
    EXPECT_EQ(RgbColor(0xFF0000), RgbColor(QColor::fromRgb(0xFF0000)));
    EXPECT_TRUE(RgbColor(QColor::fromRgb(0x00FF00)));
    EXPECT_EQ(RgbColor(0x00FF00), RgbColor(QColor::fromRgb(0x00FF00)));
    EXPECT_TRUE(RgbColor(QColor::fromRgb(0x0000FF)));
    EXPECT_EQ(RgbColor(0x0000FF), RgbColor(QColor::fromRgb(0x0000FF)));
    EXPECT_TRUE(RgbColor(QColor::fromRgb(0xFFFFFF)));
    EXPECT_EQ(RgbColor(0xFFFFFF), RgbColor(QColor::fromRgb(0xFFFFFF)));
}

TEST(RgbColorTest, FromQColorWithAlpha) {
    EXPECT_TRUE(RgbColor(QColor::fromRgba(0xAA000000)));
    EXPECT_EQ(RgbColor(0x000000), RgbColor(QColor::fromRgba(0xAA000000)));
    EXPECT_TRUE(RgbColor(QColor::fromRgba(0xAAFF0000)));
    EXPECT_EQ(RgbColor(0xFF0000), RgbColor(QColor::fromRgba(0xAAFF0000)));
    EXPECT_TRUE(RgbColor(QColor::fromRgba(0xAA00FF00)));
    EXPECT_EQ(RgbColor(0x00FF00), RgbColor(QColor::fromRgba(0xAA00FF00)));
    EXPECT_TRUE(RgbColor(QColor::fromRgba(0xAA0000FF)));
    EXPECT_EQ(RgbColor(0x0000FF), RgbColor(QColor::fromRgba(0xAA0000FF)));
    EXPECT_TRUE(RgbColor(QColor::fromRgba(0xAAFFFFFF)));
    EXPECT_EQ(RgbColor(0xFFFFFF), RgbColor(QColor::fromRgba(0xAAFFFFFF)));
}

TEST(RgbColorTest, ToQColor) {
    EXPECT_EQ(QColor(), RgbColor().toQColor());
    EXPECT_EQ(QColor::fromRgba(0xAABBCCDD),
            RgbColor().toQColor(QColor::fromRgba(0xAABBCCDD)));
    EXPECT_EQ(QColor::fromRgb(0x123456),
            RgbColor(QColor::fromRgba(0xAA123456)).toQColor(QColor::fromRgba(0xAABBCCDD)));
}

} // namespace mixxx
