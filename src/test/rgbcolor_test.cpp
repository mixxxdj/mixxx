#include "util/color/rgbcolor.h"

#include <gtest/gtest.h>

#include "test/mixxxtest.h"

namespace mixxx {

TEST(RgbColorTest, OptionalRgbColorFromInvalidQColor) {
    EXPECT_FALSE(RgbColor::optional(QColor()));
    EXPECT_EQ(RgbColor::nullopt(), RgbColor::optional(QColor()));
}

TEST(RgbColorTest, OptionalRgbColorFromQColorWithoutAlpha) {
    EXPECT_TRUE(RgbColor::optional(QColor::fromRgb(0x000000)));
    EXPECT_EQ(0x000000, *RgbColor::optional(QColor::fromRgb(0x000000)));
    EXPECT_TRUE(RgbColor::optional(QColor::fromRgb(0xFF0000)));
    EXPECT_EQ(RgbColor::optional(0xFF0000), RgbColor::optional(QColor::fromRgb(0xFF0000)));
    EXPECT_TRUE(RgbColor::optional(QColor::fromRgb(0x00FF00)));
    EXPECT_EQ(RgbColor::optional(0x00FF00), RgbColor::optional(QColor::fromRgb(0x00FF00)));
    EXPECT_TRUE(RgbColor::optional(QColor::fromRgb(0x0000FF)));
    EXPECT_EQ(RgbColor::optional(0x0000FF), RgbColor::optional(QColor::fromRgb(0x0000FF)));
    EXPECT_TRUE(RgbColor::optional(QColor::fromRgb(0xFFFFFF)));
    EXPECT_EQ(RgbColor::optional(0xFFFFFF), RgbColor::optional(QColor::fromRgb(0xFFFFFF)));
}

TEST(RgbColorTest, OptionalRgbColorFromQColorWithAlpha) {
    EXPECT_TRUE(RgbColor::optional(QColor::fromRgba(0xAA000000)));
    EXPECT_EQ(RgbColor::optional(0x000000), RgbColor::optional(QColor::fromRgba(0xAA000000)));
    EXPECT_TRUE(RgbColor::optional(QColor::fromRgba(0xAAFF0000)));
    EXPECT_EQ(RgbColor::optional(0xFF0000), RgbColor::optional(QColor::fromRgba(0xAAFF0000)));
    EXPECT_TRUE(RgbColor::optional(QColor::fromRgba(0xAA00FF00)));
    EXPECT_EQ(RgbColor::optional(0x00FF00), RgbColor::optional(QColor::fromRgba(0xAA00FF00)));
    EXPECT_TRUE(RgbColor::optional(QColor::fromRgba(0xAA0000FF)));
    EXPECT_EQ(RgbColor::optional(0x0000FF), RgbColor::optional(QColor::fromRgba(0xAA0000FF)));
    EXPECT_TRUE(RgbColor::optional(QColor::fromRgba(0xAAFFFFFF)));
    EXPECT_EQ(RgbColor::optional(0xFFFFFF), RgbColor::optional(QColor::fromRgba(0xAAFFFFFF)));
}

TEST(RgbColorTest, RgbColorToQColor) {
    EXPECT_EQ(QColor::fromRgb(0x123456), toQColor(RgbColor(0x123456)));
}

TEST(RgbColorTest, OptionalRgbColorToQColor) {
    EXPECT_EQ(QColor(), toQColor(RgbColor::nullopt()));
    EXPECT_EQ(QColor::fromRgba(0xAABBCCDD),
            toQColor(RgbColor::nullopt(), QColor::fromRgba(0xAABBCCDD)));
    EXPECT_EQ(QColor::fromRgb(0x123456),
            toQColor(RgbColor::optional(QColor::fromRgba(0xAA123456))));
    EXPECT_EQ(QColor::fromRgb(0x123456),
            toQColor(RgbColor::optional(QColor::fromRgba(0xAA123456)), QColor::fromRgba(0xAABBCCDD)));
}

TEST(RgbColorTest, RgbColorToQVariant) {
    EXPECT_EQ(QVariant(0x123456), toQVariant(RgbColor(0x123456)));
}

TEST(RgbColorTest, OptionalRgbColorToQVariant) {
    EXPECT_EQ(QVariant(), toQVariant(RgbColor::nullopt()));
    EXPECT_EQ(QVariant(0x123456),
            toQVariant(RgbColor::optional(QColor::fromRgba(0xAA123456))));
}

TEST(RgbColorTest, toQString) {
    EXPECT_EQ(QString("#123456"), toQString(RgbColor(0x123456)));
}

TEST(RgbColorTest, toNullableQString) {
    EXPECT_EQ(QString(), toQString(RgbColor::nullopt()));
    EXPECT_EQ(QString("#123456"),
            toQString(RgbColor::optional(QColor::fromRgba(0xAA123456))));
}

} // namespace mixxx
