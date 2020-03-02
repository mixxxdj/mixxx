#pragma once

#include "preferences/usersettings.h"
#include "util/color/colorpalette.h"

class ColorPaletteSettings {
  public:
    explicit ColorPaletteSettings(UserSettingsPointer pConfig)
            : m_pConfig(pConfig) {
    }

    ColorPalette getHotcueColorPalette() const;
    void setHotcueColorPalette(const ColorPalette& colorPalette);

    ColorPalette getTrackColorPalette() const;
    void setTrackColorPalette(const ColorPalette& colorPalette);

  private:
    ColorPalette getColorPalette(const QString& group, const ColorPalette& defaultPalette) const;
    void setColorPalette(const QString& group, const ColorPalette& colorPalette);

    void removePalette(const QString& group);

    ConfigKey keyForIndex(const QString& group, int index) {
        return ConfigKey(group, QString::number(index));
    }

    UserSettingsPointer m_pConfig;
};
