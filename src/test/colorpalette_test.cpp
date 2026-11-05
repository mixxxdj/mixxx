#include "util/color/colorpalette.h"

#include <gtest/gtest.h>

#include "test/mixxxtest.h"
#include "util/color/predefinedcolorpalettes.h"
#include "util/color/rgbcolor.h"

class ColorPaletteTest : public MixxxTest {};

TEST_F(ColorPaletteTest, NextColor) {
    const ColorPalette palette = mixxx::PredefinedColorPalettes::kDefaultHotcueColorPalette;
    ASSERT_TRUE(palette.size() >= 1);
    ASSERT_EQ(palette.nextColor(palette.at(0)), palette.at(1));
    ASSERT_EQ(palette.nextColor(palette.at(palette.size() - 1)), palette.at(0));
}

TEST_F(ColorPaletteTest, PreviousColor) {
    const ColorPalette palette = mixxx::PredefinedColorPalettes::kDefaultHotcueColorPalette;
    ASSERT_TRUE(palette.size() >= 1);
    ASSERT_EQ(palette.previousColor(palette.at(1)), palette.at(0));
    ASSERT_EQ(palette.previousColor(palette.at(0)), palette.at(palette.size() - 1));
}

TEST_F(ColorPaletteTest, NextAndPreviousColorRoundtrip) {
    const ColorPalette palette = mixxx::PredefinedColorPalettes::kDefaultHotcueColorPalette;
    ASSERT_TRUE(palette.size() >= 1);
    ASSERT_EQ(palette.nextColor(palette.previousColor(palette.at(0))), palette.at(0));
    ASSERT_EQ(palette.nextColor(palette.previousColor(palette.at(palette.size() - 1))), palette.at(palette.size() - 1));
    ASSERT_EQ(palette.previousColor(palette.nextColor(palette.at(0))), palette.at(0));
    ASSERT_EQ(palette.previousColor(palette.nextColor(palette.at(palette.size() - 1))), palette.at(palette.size() - 1));
}

// The sort key is used both by the `mixxx_hue()` SQL function and by
// BaseTrackCache::compareColumnValues(), so that sorting the library by
// color yields the same order in SQL and in C++.
TEST_F(ColorPaletteTest, RgbColorSortKeyOrdersByHue) {
    // Red (hue 0), green (hue 120), blue (hue 240).
    const auto red = mixxx::RgbColor::optional(0xFF0000);
    const auto green = mixxx::RgbColor::optional(0x00FF00);
    const auto blue = mixxx::RgbColor::optional(0x0000FF);

    ASSERT_LT(mixxx::RgbColor::sortKey(red), mixxx::RgbColor::sortKey(green));
    ASSERT_LT(mixxx::RgbColor::sortKey(green), mixxx::RgbColor::sortKey(blue));
}

TEST_F(ColorPaletteTest, RgbColorSortKeyIsIndependentOfBrightness) {
    // Dark and bright variants of the same hue share the same hue bucket, so
    // they are grouped together and are not separated by other hues. Within a
    // bucket the order is determined by the color code.
    const auto darkRed = mixxx::RgbColor::optional(0x800000);
    const auto brightRed = mixxx::RgbColor::optional(0xFF0000);
    const auto green = mixxx::RgbColor::optional(0x00FF00);

    ASSERT_LT(mixxx::RgbColor::sortKey(darkRed), mixxx::RgbColor::sortKey(green));
    ASSERT_LT(mixxx::RgbColor::sortKey(brightRed), mixxx::RgbColor::sortKey(green));
    ASSERT_LT(mixxx::RgbColor::sortKey(darkRed), mixxx::RgbColor::sortKey(brightRed));
}

TEST_F(ColorPaletteTest, RgbColorSortKeyPlacesAchromaticColorsLast) {
    // QColor::hue() is undefined (-1) for achromatic colors. Those must
    // sort after all chromatic ones instead of interleaving with red.
    // Within the achromatic group the order follows the color code.
    const auto blue = mixxx::RgbColor::optional(0x0000FF);
    const auto black = mixxx::RgbColor::optional(0x000000);
    const auto grey = mixxx::RgbColor::optional(0x808080);
    const auto white = mixxx::RgbColor::optional(0xFFFFFF);

    ASSERT_LT(mixxx::RgbColor::sortKey(blue), mixxx::RgbColor::sortKey(black));
    ASSERT_LT(mixxx::RgbColor::sortKey(black), mixxx::RgbColor::sortKey(grey));
    ASSERT_LT(mixxx::RgbColor::sortKey(grey), mixxx::RgbColor::sortKey(white));
}

TEST_F(ColorPaletteTest, RgbColorSortKeyPlacesUnsetColorLast) {
    const auto achromatic = mixxx::RgbColor::optional(0xFFFFFF);
    const auto unset = mixxx::RgbColor::nullopt();

    ASSERT_LT(mixxx::RgbColor::sortKey(achromatic),
            mixxx::RgbColor::sortKey(unset));
}

TEST_F(ColorPaletteTest, RgbColorSortKeyIsATotalOrder) {
    // The library sorts incrementally via binary search, which requires a
    // strict weak ordering. Hence equal colors must compare equal, and
    // different colors of the same hue must still be ordered.
    const auto color = mixxx::RgbColor::optional(0x123456);
    ASSERT_EQ(mixxx::RgbColor::sortKey(color), mixxx::RgbColor::sortKey(color));
    ASSERT_EQ(mixxx::RgbColor::sortKey(mixxx::RgbColor::optional(0x123456)),
            mixxx::RgbColor::sortKey(mixxx::RgbColor::optional(0x123456)));

    const auto sameHue1 = mixxx::RgbColor::optional(0xFF0000);
    const auto sameHue2 = mixxx::RgbColor::optional(0xFE0000);
    ASSERT_NE(mixxx::RgbColor::sortKey(sameHue1), mixxx::RgbColor::sortKey(sameHue2));
}
