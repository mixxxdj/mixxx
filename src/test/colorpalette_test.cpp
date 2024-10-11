#include "util/color/colorpalette.h"

#include <gtest/gtest.h>

#include "test/mixxxtest.h"
#include "util/color/predefinedcolorpalettes.h"

class ColorPaletteTest : public MixxxTest {
  protected:
    ColorPalette palette{mixxx::predefinedcolorpalettes::get().defaultHotcueColorPalette};
    void SetUp() override {
        ASSERT_TRUE(palette.size() >= 1);
    }
};

TEST_F(ColorPaletteTest, NextColor) {
    ASSERT_EQ(palette.nextColor(palette.at(0)), palette.at(1));
    ASSERT_EQ(palette.nextColor(palette.at(palette.size() - 1)), palette.at(0));
}

TEST_F(ColorPaletteTest, PreviousColor) {
    ASSERT_EQ(palette.previousColor(palette.at(1)), palette.at(0));
    ASSERT_EQ(palette.previousColor(palette.at(0)), palette.at(palette.size() - 1));
}

TEST_F(ColorPaletteTest, NextAndPreviousColorRoundtrip) {
    ASSERT_EQ(palette.nextColor(palette.previousColor(palette.at(0))), palette.at(0));
    ASSERT_EQ(palette.nextColor(palette.previousColor(palette.at(palette.size() - 1))), palette.at(palette.size() - 1));
    ASSERT_EQ(palette.previousColor(palette.nextColor(palette.at(0))), palette.at(0));
    ASSERT_EQ(palette.previousColor(palette.nextColor(palette.at(palette.size() - 1))), palette.at(palette.size() - 1));
}
