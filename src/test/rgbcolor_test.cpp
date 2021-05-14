#include "util/color/rgbcolor.h"

#include <gtest/gtest.h>

#include "test/mixxxtest.h"
#include "control/convert.h"

namespace mixxx {

TEST(RgbColorTest, fromInvalidQColor) {
    EXPECT_FALSE(RgbColor::fromQColor(QColor()));
    EXPECT_EQ(RgbColor::nullopt(), RgbColor::fromQColor(QColor()));
}

TEST(RgbColorTest, fromQColorWithoutAlpha) {
    EXPECT_TRUE(RgbColor::fromQColor(QColor::fromRgb(0x000000)));
    EXPECT_EQ(RgbColor::optional(0x000000), RgbColor::fromQColor(QColor::fromRgb(0x000000)));
    EXPECT_TRUE(RgbColor::fromQColor(QColor::fromRgb(0xFF0000)));
    EXPECT_EQ(RgbColor::optional(0xFF0000), RgbColor::fromQColor(QColor::fromRgb(0xFF0000)));
    EXPECT_TRUE(RgbColor::fromQColor(QColor::fromRgb(0x00FF00)));
    EXPECT_EQ(RgbColor::optional(0x00FF00), RgbColor::fromQColor(QColor::fromRgb(0x00FF00)));
    EXPECT_TRUE(RgbColor::fromQColor(QColor::fromRgb(0x0000FF)));
    EXPECT_EQ(RgbColor::optional(0x0000FF), RgbColor::fromQColor(QColor::fromRgb(0x0000FF)));
    EXPECT_TRUE(RgbColor::fromQColor(QColor::fromRgb(0xFFFFFF)));
    EXPECT_EQ(RgbColor::optional(0xFFFFFF), RgbColor::fromQColor(QColor::fromRgb(0xFFFFFF)));
}

TEST(RgbColorTest, fromQColorWithAlpha) {
    EXPECT_TRUE(RgbColor::fromQColor(QColor::fromRgba(0xAA000000)));
    EXPECT_EQ(RgbColor::optional(0x000000), RgbColor::fromQColor(QColor::fromRgba(0xAA000000)));
    EXPECT_TRUE(RgbColor::fromQColor(QColor::fromRgba(0xAAFF0000)));
    EXPECT_EQ(RgbColor::optional(0xFF0000), RgbColor::fromQColor(QColor::fromRgba(0xAAFF0000)));
    EXPECT_TRUE(RgbColor::fromQColor(QColor::fromRgba(0xAA00FF00)));
    EXPECT_EQ(RgbColor::optional(0x00FF00), RgbColor::fromQColor(QColor::fromRgba(0xAA00FF00)));
    EXPECT_TRUE(RgbColor::fromQColor(QColor::fromRgba(0xAA0000FF)));
    EXPECT_EQ(RgbColor::optional(0x0000FF), RgbColor::fromQColor(QColor::fromRgba(0xAA0000FF)));
    EXPECT_TRUE(RgbColor::fromQColor(QColor::fromRgba(0xAAFFFFFF)));
    EXPECT_EQ(RgbColor::optional(0xFFFFFF), RgbColor::fromQColor(QColor::fromRgba(0xAAFFFFFF)));
}

TEST(RgbColorTest, toQColor) {
    EXPECT_EQ(QColor::fromRgb(0x123456),
            RgbColor::toQColor(RgbColor(0x123456)));
}

TEST(RgbColorTest, toQColorOptional) {
    EXPECT_EQ(QColor(),
            RgbColor::toQColor(RgbColor::nullopt()));
    EXPECT_EQ(QColor::fromRgba(0xAABBCCDD),
            RgbColor::toQColor(RgbColor::nullopt(), QColor::fromRgba(0xAABBCCDD)));
    EXPECT_EQ(QColor::fromRgb(0x123456),
            RgbColor::toQColor(RgbColor::fromQColor(QColor::fromRgba(0xAA123456))));
    EXPECT_EQ(QColor::fromRgb(0x123456),
            RgbColor::toQColor(RgbColor::fromQColor(QColor::fromRgba(0xAA123456)), QColor::fromRgba(0xAABBCCDD)));
}

TEST(RgbColorTest, toQVariant) {
    EXPECT_EQ(QVariant(0x123456),
            RgbColor::toQVariant(RgbColor(0x123456)));
}

TEST(RgbColorTest, toQVariantOptional) {
    EXPECT_EQ(QVariant(),
            RgbColor::toQVariant(RgbColor::nullopt()));
    EXPECT_EQ(QVariant(0x123456),
            RgbColor::toQVariant(RgbColor::fromQColor(QColor::fromRgba(0xAA123456))));
}

TEST(RgbColorTest, toQString) {
    EXPECT_EQ(QString("#123456"),
            RgbColor::toQString(RgbColor(0x123456)));
}

TEST(RgbColorTest, toQStringOptional) {
    EXPECT_EQ(QString(),
            RgbColor::toQString(RgbColor::nullopt()));
    EXPECT_EQ(QString("None"),
            RgbColor::toQString(RgbColor::nullopt(), QStringLiteral("None")));
    EXPECT_EQ(QString("#123456"),
            RgbColor::toQString(RgbColor::fromQColor(QColor::fromRgba(0xAA123456))));
    EXPECT_EQ(QString("#123456"),
            RgbColor::toQString(RgbColor::fromQColor(QColor::fromRgba(0xAA123456)), QStringLiteral("None")));
}

TEST(RgbColorTest, fromControlValue) {
    EXPECT_EQ(RgbColor::nullopt(),
            control::doubleToRgbColor(control::kInvalidRgbColor));
    EXPECT_EQ(RgbColor::optional(0),
            control::doubleToRgbColor(0));
    EXPECT_EQ(RgbColor::optional(0xFEDCBA),
            control::doubleToRgbColor(0xFEDCBA));
}

TEST(RgbColorTest, toControlValue) {
    EXPECT_EQ(control::kInvalidRgbColor,
            control::doubleFromRgbColor(RgbColor::nullopt()));
    EXPECT_EQ(0.0,
            control::doubleFromRgbColor(RgbColor(0)));
    EXPECT_EQ(0.0,
            control::doubleFromRgbColor(RgbColor::optional(0)));
    EXPECT_EQ(16702650.0,
            control::doubleFromRgbColor(RgbColor::optional(0xFEDCBA)));
    EXPECT_EQ(16702650.0,
            control::doubleFromRgbColor(RgbColor::optional(0xFEDCBA)));
}

} // namespace mixxx
