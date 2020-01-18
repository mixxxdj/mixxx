#include <gtest/gtest.h>
#include <QColor>

#include "test/mixxxtest.h"

#include <preferences/hotcuecolorpalettesettings.h>
#include <util/color/colorpalette.h>

class ColorConfigTest : public MixxxTest {};

TEST_F(ColorConfigTest, SavingColorWithoutAlpha) {
    ConfigKey key("[Color]", "color");
    QColor originalColor("#FF9900");
    config()->setValue(key, originalColor);
    saveAndReloadConfig();
    QColor colorFromConfig = config()->getValue(key);
    ASSERT_EQ(originalColor, colorFromConfig);
}

TEST_F(ColorConfigTest, SavingColorWithAlpha) {
    ConfigKey key("[Color]", "color");
    QColor originalColor("#66FF9900");
    QColor expectedColor("#FF9900");
    config()->setValue(key, originalColor);
    saveAndReloadConfig();
    QColor colorFromConfig = config()->getValue(key);
    ASSERT_EQ(expectedColor, colorFromConfig);
}

TEST_F(ColorConfigTest, GetDefaultColorWhenNoStoredColor) {
    ConfigKey key("[Color]", "color");
    QColor defaultColor("#FF9900");
    QColor colorFromConfig = config()->getValue(key, defaultColor);
    ASSERT_EQ(defaultColor, colorFromConfig);
}

TEST_F(ColorConfigTest, SaveColorPalette) {
    HotcueColorPaletteSettings colorPaletteSettings(config());
    ColorPalette originalColorPalette(QList<QRgb>{
            0xFF9900,
            0xFF9900,
            0x000000,
            0xFFFFFF
    });
    ConfigKey key("[ColorPalette]", "colorPalette");
    colorPaletteSettings.setHotcueColorPalette(originalColorPalette);
    saveAndReloadConfig();
    ColorPalette colorPaletteFromConfig =
            colorPaletteSettings.getHotcueColorPalette();
    ASSERT_EQ(originalColorPalette, colorPaletteFromConfig);
}

TEST_F(ColorConfigTest, ReplaceColorPalette) {
    HotcueColorPaletteSettings colorPaletteSettings(config());
    ColorPalette colorPalette1(QList<QRgb>{
            0xFF9900,
            0xFF9900,
            0x000000,
            0xFFFFFF
    });
    ColorPalette colorPalette2(QList<QRgb>{
            0x0000FF,
            0xFF0000
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
    HotcueColorPaletteSettings colorPaletteSettings(config());
    ColorPalette colorPaletteFromConfig =
            colorPaletteSettings.getHotcueColorPalette();
    ASSERT_EQ(ColorPalette::mixxxHotcuesPalette, colorPaletteFromConfig);
}
