#include <gtest/gtest.h>

#include "preferences/colorpalettesettings.h"
#include "test/mixxxtest.h"
#include "util/color/colorpalette.h"
#include "util/color/predefinedcolorpalettes.h"
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
    ColorPalette originalColorPalette(
            "SaveColorPaletteTest", QList<mixxx::RgbColor>{
                                            mixxx::RgbColor(0x66FF99),
                                            mixxx::RgbColor(0xFF9900),
                                            mixxx::RgbColor(0x000000),
                                            mixxx::RgbColor(0xFFFFFF),
                                    });
    colorPaletteSettings.setHotcueColorPalette(originalColorPalette);
    saveAndReloadConfig();
    ColorPalette colorPaletteFromConfig =
            colorPaletteSettings.getHotcueColorPalette();
    ASSERT_EQ(originalColorPalette, colorPaletteFromConfig);
}

TEST_F(ColorConfigTest, ReplaceColorPalette) {
    ColorPaletteSettings colorPaletteSettings(config());
    ColorPalette colorPalette1(
            "ReplaceColorPaletteTest", QList<mixxx::RgbColor>{
                                               mixxx::RgbColor(0x66FF99),
                                               mixxx::RgbColor(0xFF9900),
                                               mixxx::RgbColor(0x000000),
                                               mixxx::RgbColor(0xFFFFFF),
                                       });
    ColorPalette colorPalette2(
            "ReplaceColorPaletteTest", QList<mixxx::RgbColor>{
                                               mixxx::RgbColor(0x0000FF),
                                               mixxx::RgbColor(0xFF0000),
                                       });
    colorPaletteSettings.setHotcueColorPalette(colorPalette1);
    saveAndReloadConfig();
    colorPaletteSettings.setHotcueColorPalette(colorPalette2);
    saveAndReloadConfig();
    ColorPalette colorPaletteFromConfig =
            colorPaletteSettings.getHotcueColorPalette();
    ASSERT_EQ(colorPalette2, colorPaletteFromConfig);
}

TEST_F(ColorConfigTest, LoadSavePalettes) {
    const QString kName1 = QStringLiteral("Custom Palette No. 1");
    const QString kName2 = QStringLiteral("My Custom Palette 2");
    const QString kName3 = QStringLiteral("I'm blue, da ba dee");
    ColorPaletteSettings colorPaletteSettings(config());
    ColorPalette colorPalette1(
            kName1, QList<mixxx::RgbColor>{
                            mixxx::RgbColor(0x66FF99),
                            mixxx::RgbColor(0xFF9900),
                            mixxx::RgbColor(0x000000),
                            mixxx::RgbColor(0xFFFFFF),
                    });
    colorPaletteSettings.setColorPalette(colorPalette1.getName(), colorPalette1);
    ColorPalette colorPalette2(
            kName2, QList<mixxx::RgbColor>{
                            mixxx::RgbColor(0x0000FF),
                            mixxx::RgbColor(0xFF0000),
                    });
    colorPaletteSettings.setColorPalette(colorPalette2.getName(), colorPalette2);
    ColorPalette colorPalette3(
            kName3, QList<mixxx::RgbColor>{
                            mixxx::RgbColor(0x0000FF),
                            mixxx::RgbColor(0x123456),
                            mixxx::RgbColor(0x000080),
                    });
    colorPaletteSettings.setColorPalette(colorPalette3.getName(), colorPalette3);
    saveAndReloadConfig();
    QSet<QString> expectedNames{
            kName1,
            kName2,
            kName3,
    };
    ASSERT_EQ(expectedNames, colorPaletteSettings.getColorPaletteNames());
}

TEST_F(ColorConfigTest, DefaultColorPalette) {
    ColorPaletteSettings colorPaletteSettings(config());
    const auto& kPalettes = mixxx::predefinedcolorpalettes::get();
    ASSERT_EQ(kPalettes.defaultHotcueColorPalette,
            colorPaletteSettings.getHotcueColorPalette());
    ASSERT_EQ(kPalettes.defaultTrackColorPalette,
            colorPaletteSettings.getTrackColorPalette());
}
