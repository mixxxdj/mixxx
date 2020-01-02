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
    ConfigKey autoHotcueColorsKey("[Controls]", "HotcueDefaultColorIndex");
    //  QRgb(0xf36100) library icons orange
    int index = m_pConfig->getValue(autoHotcueColorsKey, -1);
    if (index == -2) {
        index = hotcue;
    }

    if (index < 0) {
        // -1 or invalid hotcue
        return QRgb(0xf36100); // library icons orange
    }

    auto hotcueColorPalette = getHotcueColorPalette();
    auto colors = hotcueColorPalette.m_colorList;
    return colors.at(index % colors.count());
}
