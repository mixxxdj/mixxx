#include "preferences/hotcuecolorpalettesettings.h"

const QString HotcueColorPaletteSettings::sGroup = "[HotcueColorPalette]";

ColorPalette HotcueColorPaletteSettings::getHotcueColorPalette() const {
    QList<QRgb> colorList;
    for (const ConfigKey& key : m_pConfig->getKeysWithGroup(sGroup)) {
        QColor color = m_pConfig->getValue<QColor>(key);
        if (color.isValid()) {
            colorList.append(color.rgb());
        }
    }

    // If no palette is defined in the settings, we use the default one.
    if (colorList.isEmpty()) {
        return ColorPalette::mixxxHotcuesPalette;
    }

    return colorList;
}

void HotcueColorPaletteSettings::setHotcueColorPalette(
        const ColorPalette& colorPalette) {
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

QRgb HotcueColorPaletteSettings::getDefaultColor(int hotcue) const {
    if (hotcue >= 0) {
        auto hotcueColorPalette = getHotcueColorPalette();
        ConfigKey autoHotcueColorsKey("[Controls]", "auto_hotcue_colors");
        if (m_pConfig->getValue(autoHotcueColorsKey, false)) {
            auto colors = hotcueColorPalette.m_colorList;
            return colors.at(hotcue % colors.count());
        }
    }
    return QRgb(0xf36100); // library icons orange
}
