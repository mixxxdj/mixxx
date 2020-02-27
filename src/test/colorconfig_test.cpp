#include <gtest/gtest.h>

#include "preferences/colorpalettesettings.h"
#include "test/mixxxtest.h"
#include "util/color/colorpalette.h"
#include "util/color/rgbcolor.h"

class ColorConfigTest : public MixxxTest {};

TEST_F(ColorConfigTest, SavingColor) {
    ConfigKey key("[Color]", "color");
    mixxx::RgbColor originalColor(0xFF9900);
    config()->setValue(key, originalColor);
    saveAndReloadConfig();
    mixxx::RgbColor colorFromConfig = config()->getValue(key, mixxx::RgbColor(0));
    ASSERT_EQ(originalColor, colorFromConfig);
}

TEST_F(ColorConfigTest, GetDefaultColorWhenNoStoredColor) {
    ConfigKey key("[Color]", "color");
    mixxx::RgbColor defaultColor(0x66FF99);
    mixxx::RgbColor colorFromConfig = config()->getValue(key, defaultColor);
    ASSERT_EQ(defaultColor, colorFromConfig);
}

TEST_F(ColorConfigTest, SaveColorPalette) {
    ColorPaletteSettings colorPaletteSettings(config());
    ColorPalette originalColorPalette(QList<mixxx::RgbColor>{
            mixxx::RgbColor(0x66FF99),
            mixxx::RgbColor(0xFF9900),
            mixxx::RgbColor(0x000000),
            mixxx::RgbColor(0xFFFFFF),
    });
    ConfigKey key("[ColorPalette]", "colorPalette");
    colorPaletteSettings.setHotcueColorPalette(originalColorPalette);
    saveAndReloadConfig();
    ColorPalette colorPaletteFromConfig =
            colorPaletteSettings.getHotcueColorPalette();
    ASSERT_EQ(originalColorPalette, colorPaletteFromConfig);
}

TEST_F(ColorConfigTest, ReplaceColorPalette) {
    ColorPaletteSettings colorPaletteSettings(config());
    ColorPalette colorPalette1(QList<mixxx::RgbColor>{
            mixxx::RgbColor(0x66FF99),
            mixxx::RgbColor(0xFF9900),
            mixxx::RgbColor(0x000000),
            mixxx::RgbColor(0xFFFFFF),
    });
    ColorPalette colorPalette2(QList<mixxx::RgbColor>{
            mixxx::RgbColor(0x0000FF),
            mixxx::RgbColor(0xFF0000),
    });
    ConfigKey key("[ColorPalette]", "colorPalette");
    colorPaletteSettings.setHotcueColorPalette(colorPalette1);
    saveAndReloadConfig();
    colorPaletteSettings.setHotcueColorPalette(colorPalette2);
    saveAndReloadConfig();
    ColorPalette colorPaletteFromConfig =
            colorPaletteSettings.getHotcueColorPalette();
    ASSERT_EQ(colorPalette2, colorPaletteFromConfig);
}

TEST_F(ColorConfigTest, DefaultColorPalette) {
    ColorPaletteSettings colorPaletteSettings(config());
    ColorPalette colorPaletteFromConfig =
            colorPaletteSettings.getHotcueColorPalette();
    ASSERT_EQ(ColorPalette::mixxxHotcuePalette, colorPaletteFromConfig);
}
