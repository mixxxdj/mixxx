#pragma once

#include "preferences/usersettings.h"
#include "util/color/colorpalette.h"

// Saves ColorPalettes to and loads ColorPalettes from the mixxx.cfg file
class ColorPaletteSettings {
  public:
    explicit ColorPaletteSettings(UserSettingsPointer pConfig)
            : m_pConfig(pConfig) {
    }

    ColorPalette getHotcueColorPalette(const QString& name) const;
    ColorPalette getHotcueColorPalette() const;
    void setHotcueColorPalette(const ColorPalette& colorPalette);

    ColorPalette getTrackColorPalette(const QString& name) const;
    ColorPalette getTrackColorPalette() const;
    void setTrackColorPalette(const ColorPalette& colorPalette);

    ColorPalette getKeyColorPalette(const QString& name) const;
    ColorPalette getConfigKeyColorPalette() const;
    void setKeyColorPalette(const ColorPalette& colorPalette);

    ColorPalette getColorPalette(
            const QString& name,
            const ColorPalette& defaultPalette) const;
    void setColorPalette(const QString& name, const ColorPalette& colorPalette);
    void removePalette(const QString& name);
    QSet<QString> getColorPaletteNames() const;

    DEFINE_PREFERENCE_HELPERS(KeyColorsEnabled, bool, "[Config]", "key_colors_enabled", true);

  private:
    UserSettingsPointer m_pConfig;
};
