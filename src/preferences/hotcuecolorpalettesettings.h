#pragma once

#include "preferences/usersettings.h"
#include "util/color/colorpalette.h"

class HotcueColorPaletteSettings {
  public:
    explicit HotcueColorPaletteSettings(const UserSettingsPointer& pConfig)
            : m_pConfig(pConfig) {
    }

    ColorPalette getHotcueColorPalette() const;

    void setHotcueColorPalette(const ColorPalette& colorPalette);

  private:
    static const QString sGroup;

    void removePalette();

    ConfigKey keyForIndex(int index) {
        return ConfigKey(sGroup, QString::number(index));
    }

    UserSettingsPointer m_pConfig;
};
