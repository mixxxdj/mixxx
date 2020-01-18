#pragma once

#include "preferences/usersettings.h"
#include "util/color/colorpalette.h"

class HotcueColorPaletteSettings {
  public:

    enum class PaletteIndex {
        SkinDefault = -3,
        ByHotCueNumber = -2,
        OffPalette = -1
    };

    explicit HotcueColorPaletteSettings(const UserSettingsPointer& pConfig)
            : m_pConfig(pConfig) {
    }

    ColorPalette getHotcueColorPalette() const;

    void setHotcueColorPalette(const ColorPalette& colorPalette);

    QRgb getDefaultColor(int hotcue, bool* keepDefault = nullptr) const;

    QRgb offPaletteDefaultColor() const;

  private:
    static const QString sGroup;

    void removePalette();

    ConfigKey keyForIndex(int index) {
        return ConfigKey(sGroup, QString::number(index));
    }

    UserSettingsPointer m_pConfig;
};
