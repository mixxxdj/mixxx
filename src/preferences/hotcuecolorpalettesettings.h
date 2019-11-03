#pragma once

#include "preferences/usersettings.h"
#include "util/color/hotcuecolorpalette.h"

class HotcueColorPaletteSettings {
  public:
    explicit HotcueColorPaletteSettings(UserSettingsPointer pConfig)
            : m_pConfig(pConfig) {
    }

    HotcueColorPalette getHotcueColorPalette() const;

    void setHotcueColorPalette(const HotcueColorPalette& colorPalette);

  private:
    static const QString sGroup;

    void removePalette();

    ConfigKey keyForIndex(int index) {
        return ConfigKey(sGroup, QString::number(index));
    }

    UserSettingsPointer m_pConfig;
};
