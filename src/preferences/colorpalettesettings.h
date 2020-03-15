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

    ColorPalette getColorPalette(const QString& name, const ColorPalette& defaultPalette) const;
    void setColorPalette(const QString& name, const ColorPalette& colorPalette);
    void removePalette(const QString& name);
    QSet<QString> getColorPaletteNames() const;

  private:
    UserSettingsPointer m_pConfig;
};
