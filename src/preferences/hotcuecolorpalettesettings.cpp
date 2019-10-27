#include "preferences/hotcuecolorpalettesettings.h"

const QString HotcueColorPaletteSettings::sGroup = "[HotcueColorPalette]";

HotcueColorPalette HotcueColorPaletteSettings::getHotcueColorPalette() const {
    QList<QColor> colorList;
    for (const ConfigKey& key : m_pConfig->getKeysWithGroup(sGroup)) {
        QColor color = m_pConfig->getValue<QColor>(key);
        if (color.isValid()) {
            colorList.append(color);
        }
    }

    // If no palette is defined in the settings, we use the default one.
    if (colorList.isEmpty()) {
        return HotcueColorPalette::mixxxPalette;
    }

    return colorList;
}

void HotcueColorPaletteSettings::setHotcueColorPalette(
        const HotcueColorPalette& colorPalette) {
    removePalette();

    for (int index = 0; index < colorPalette.m_colorList.count(); ++index) {
        QColor color = colorPalette.m_colorList[index];
        m_pConfig->setValue<QColor>(keyForIndex(index), color);
    }
}

void HotcueColorPaletteSettings::removePalette() {
    for (const ConfigKey& key : m_pConfig->getKeysWithGroup(sGroup)) {
        m_pConfig->remove(key);
    }
}