#include "preferences/hotcuecolorpalettesettings.h"

const QString HotcueColorPaletteSettings::sGroup = "[HotcueColorPalette]";

ColorPalette HotcueColorPaletteSettings::getHotcueColorPalette() const {
    QList<mixxx::RgbColor> colorList;
    for (const ConfigKey& key : m_pConfig->getKeysWithGroup(sGroup)) {
        mixxx::RgbColor color = mixxx::RgbColor(m_pConfig->getValue<mixxx::RgbColor>(key, mixxx::RgbColor(0)));
        colorList.append(color);
    }

    // If no palette is defined in the settings, we use the default one.
    if (colorList.isEmpty()) {
        return ColorPalette::mixxxPalette;
    }

    return colorList;
}

void HotcueColorPaletteSettings::setHotcueColorPalette(
        const ColorPalette& colorPalette) {
    removePalette();

    for (int index = 0; index < colorPalette.m_colorList.count(); ++index) {
        mixxx::RgbColor color = colorPalette.m_colorList[index];
        m_pConfig->setValue<mixxx::RgbColor>(keyForIndex(index), color);
    }
}

void HotcueColorPaletteSettings::removePalette() {
    for (const ConfigKey& key : m_pConfig->getKeysWithGroup(sGroup)) {
        m_pConfig->remove(key);
    }
}
