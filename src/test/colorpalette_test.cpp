#include "util/color/colorpalette.h"

#include <gtest/gtest.h>

#include "test/mixxxtest.h"

class ColorPaletteTest : public MixxxTest {};

TEST_F(ColorPaletteTest, NextColor) {
    const ColorPalette palette = ColorPalette::mixxxHotcuePalette;
    ASSERT_TRUE(palette.size() >= 1);
    ASSERT_EQ(palette.nextColor(palette.at(0)), palette.at(1));
    ASSERT_EQ(palette.nextColor(palette.at(palette.size() - 1)), palette.at(0));
}

TEST_F(ColorPaletteTest, PreviousColor) {
    const ColorPalette palette = ColorPalette::mixxxHotcuePalette;
    ASSERT_TRUE(palette.size() >= 1);
    ASSERT_EQ(palette.previousColor(palette.at(1)), palette.at(0));
    ASSERT_EQ(palette.previousColor(palette.at(0)), palette.at(palette.size() - 1));
}

TEST_F(ColorPaletteTest, NextAndPreviousColorRoundtrip) {
    const ColorPalette palette = ColorPalette::mixxxHotcuePalette;
    ASSERT_TRUE(palette.size() >= 1);
    ASSERT_EQ(palette.nextColor(palette.previousColor(palette.at(0))), palette.at(0));
    ASSERT_EQ(palette.nextColor(palette.previousColor(palette.at(palette.size() - 1))), palette.at(palette.size() - 1));
    ASSERT_EQ(palette.previousColor(palette.nextColor(palette.at(0))), palette.at(0));
    ASSERT_EQ(palette.previousColor(palette.nextColor(palette.at(palette.size() - 1))), palette.at(palette.size() - 1));
}
