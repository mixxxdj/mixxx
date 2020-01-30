#include "util/color/rgbcolor.h"

#include <gtest/gtest.h>

#include "test/mixxxtest.h"

namespace mixxx {

TEST(OptionalRgbColorTest, DefaultIsUndefined) {
    EXPECT_FALSE(OptionalRgbColor().optional());
    EXPECT_FALSE(static_cast<bool>(OptionalRgbColor().optional()));
}

TEST(OptionalRgbColorTest, FromRgbColorCode) {
    EXPECT_TRUE(OptionalRgbColor(0x000000).optional());
    EXPECT_EQ(RgbColor(0x000000), *OptionalRgbColor(0x000000).optional());
    EXPECT_TRUE(OptionalRgbColor(0xFF0000).optional());
    EXPECT_EQ(RgbColor(0xFF0000), *OptionalRgbColor(0xFF0000).optional());
    EXPECT_TRUE(OptionalRgbColor(0x00FF00).optional());
    EXPECT_EQ(RgbColor(0x00FF00), *OptionalRgbColor(0x00FF00).optional());
    EXPECT_TRUE(OptionalRgbColor(0x0000FF).optional());
    EXPECT_EQ(RgbColor(0x0000FF), *OptionalRgbColor(0x0000FF).optional());
    EXPECT_TRUE(OptionalRgbColor(0xFFFFFF).optional());
    EXPECT_EQ(RgbColor(0xFFFFFF), *OptionalRgbColor(0xFFFFFF).optional());
}

TEST(OptionalRgbColorTest, FromOptionalRgbColorCode) {
    EXPECT_EQ(OptionalRgbColor(), OptionalRgbColor(std::nullopt));
    EXPECT_EQ(OptionalRgbColor(0x123456), OptionalRgbColor(std::make_optional(RgbColor(0x123456))));
}

TEST(OptionalRgbColorTest, FromQColor) {
    EXPECT_FALSE(OptionalRgbColor(QColor()).optional());
    EXPECT_EQ(OptionalRgbColor(), OptionalRgbColor(QColor()));
    EXPECT_TRUE(OptionalRgbColor(QColor::fromRgb(0x000000)).optional());
    EXPECT_EQ(OptionalRgbColor(0x000000), OptionalRgbColor(QColor::fromRgb(0x000000)));
    EXPECT_TRUE(OptionalRgbColor(QColor::fromRgb(0xFF0000)).optional());
    EXPECT_EQ(OptionalRgbColor(0xFF0000), OptionalRgbColor(QColor::fromRgb(0xFF0000)));
    EXPECT_TRUE(OptionalRgbColor(QColor::fromRgb(0x00FF00)).optional());
    EXPECT_EQ(OptionalRgbColor(0x00FF00), OptionalRgbColor(QColor::fromRgb(0x00FF00)));
    EXPECT_TRUE(OptionalRgbColor(QColor::fromRgb(0x0000FF)).optional());
    EXPECT_EQ(OptionalRgbColor(0x0000FF), OptionalRgbColor(QColor::fromRgb(0x0000FF)));
    EXPECT_TRUE(OptionalRgbColor(QColor::fromRgb(0xFFFFFF)).optional());
    EXPECT_EQ(OptionalRgbColor(0xFFFFFF), OptionalRgbColor(QColor::fromRgb(0xFFFFFF)));
}

TEST(OptionalRgbColorTest, FromQColorWithAlpha) {
    EXPECT_TRUE(OptionalRgbColor(QColor::fromRgba(0xAA000000)).optional());
    EXPECT_EQ(OptionalRgbColor(0x000000), OptionalRgbColor(QColor::fromRgba(0xAA000000)));
    EXPECT_TRUE(OptionalRgbColor(QColor::fromRgba(0xAAFF0000)).optional());
    EXPECT_EQ(OptionalRgbColor(0xFF0000), OptionalRgbColor(QColor::fromRgba(0xAAFF0000)));
    EXPECT_TRUE(OptionalRgbColor(QColor::fromRgba(0xAA00FF00)).optional());
    EXPECT_EQ(OptionalRgbColor(0x00FF00), OptionalRgbColor(QColor::fromRgba(0xAA00FF00)));
    EXPECT_TRUE(OptionalRgbColor(QColor::fromRgba(0xAA0000FF)).optional());
    EXPECT_EQ(OptionalRgbColor(0x0000FF), OptionalRgbColor(QColor::fromRgba(0xAA0000FF)));
    EXPECT_TRUE(OptionalRgbColor(QColor::fromRgba(0xAAFFFFFF)).optional());
    EXPECT_EQ(OptionalRgbColor(0xFFFFFF), OptionalRgbColor(QColor::fromRgba(0xAAFFFFFF)));
}

TEST(OptionalRgbColorTest, ToQColor) {
    EXPECT_EQ(QColor(), OptionalRgbColor().toQColor());
    EXPECT_EQ(QColor::fromRgba(0xAABBCCDD),
            OptionalRgbColor().toQColor(QColor::fromRgba(0xAABBCCDD)));
    EXPECT_EQ(QColor::fromRgb(0x123456),
            OptionalRgbColor(QColor::fromRgba(0xAA123456)).toQColor(QColor::fromRgba(0xAABBCCDD)));
}

} // namespace mixxx
