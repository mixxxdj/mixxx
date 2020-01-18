#pragma once

#include "preferences/usersettings.h"
#include "util/color/colorpalette.h"

class ColorPaletteSettings {
  public:
    explicit ColorPaletteSettings(QString group, UserSettingsPointer pConfig);
    ~ColorPaletteSettings();

    ColorPalette getPalette() const {
        return m_palette;
    }

    void setPalette(const ColorPalette& newPalette) {
        m_palette = newPalette;
        save();
    }

  private:
    void loadFromConfig();
    void save();

    ConfigKey keyForIndex(int index) {
        return ConfigKey(m_configGroup, QString::number(index));
    }

    QString m_configGroup;
    ColorPalette m_palette;
    UserSettingsPointer m_pConfig;
};
