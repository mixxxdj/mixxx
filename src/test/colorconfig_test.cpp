#include <gtest/gtest.h>
#include <QColor>

#include "test/mixxxtest.h"

#include <preferences/hotcuecolorpalettesettings.h>
#include <util/color/hotcuecolorpalette.h>

class ColorConfigTest : public MixxxTest {};

TEST_F(ColorConfigTest, TestSavingColorWithoutAlpha) {
    ConfigKey key("[Color]", "color");
    QColor originalColor("#FF9900");
    config()->setValue(key, originalColor);
    saveAndReloadConfig();
    QColor colorFromConfig = config()->getValue(key);
    ASSERT_EQ(originalColor, colorFromConfig);
}

TEST_F(ColorConfigTest, TestSavingColorWithAlpha) {
    ConfigKey key("[Color]", "color");
    QColor originalColor("#66FF9900");
    config()->setValue(key, originalColor);
    saveAndReloadConfig();
    QColor colorFromConfig = config()->getValue(key);
    ASSERT_EQ(originalColor, colorFromConfig);
}

TEST_F(ColorConfigTest, TestDefaultColorWhenNoStoredColor) {
    ConfigKey key("[Color]", "color");
    QColor defaultColor("#66FF9900");
    QColor colorFromConfig = config()->getValue(key, defaultColor);
    ASSERT_EQ(defaultColor, colorFromConfig);
}

TEST_F(ColorConfigTest, TestSaveColorPalette) {
    HotcueColorPaletteSettings colorPaletteSettings(config());
    HotcueColorPalette originalColorPalette(QList<QColor>{
            QColor("#66FF9900"),
            QColor("#FF9900"),
            QColor("#00000000"),
            QColor("#FFFFFF"),
    });
    ConfigKey key("[ColorPalette]", "colorPalette");
    colorPaletteSettings.setHotcueColorPalette(originalColorPalette);
    saveAndReloadConfig();
    HotcueColorPalette colorPaletteFromConfig =
            colorPaletteSettings.getHotcueColorPalette();
    ASSERT_EQ(originalColorPalette, colorPaletteFromConfig);
}

TEST_F(ColorConfigTest, TestReplaceColorPalette) {
    HotcueColorPaletteSettings colorPaletteSettings(config());
    HotcueColorPalette colorPalette1(QList<QColor>{
            QColor("#66FF9900"),
            QColor("#FF9900"),
            QColor("#00000000"),
            QColor("#FFFFFF"),
    });
    HotcueColorPalette colorPalette2(QList<QColor>{
            QColor("#0000FF"),
            QColor("#FF0000"),
    });
    ConfigKey key("[ColorPalette]", "colorPalette");
    colorPaletteSettings.setHotcueColorPalette(colorPalette1);
    saveAndReloadConfig();
    colorPaletteSettings.setHotcueColorPalette(colorPalette2);
    saveAndReloadConfig();
    HotcueColorPalette colorPaletteFromConfig =
            colorPaletteSettings.getHotcueColorPalette();
    ASSERT_EQ(colorPalette2, colorPaletteFromConfig);
}

TEST_F(ColorConfigTest, TestDefaultColorPalette) {
    HotcueColorPaletteSettings colorPaletteSettings(config());
    HotcueColorPalette colorPaletteFromConfig =
            colorPaletteSettings.getHotcueColorPalette();
    ASSERT_EQ(HotcueColorPalette::mixxxPalette, colorPaletteFromConfig);
}