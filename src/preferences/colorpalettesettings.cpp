#include "preferences/colorpalettesettings.h"

const QString ColorPaletteSettings::hotcueColorPaletteGroup = QStringLiteral("[HotcueColorPalette]");

ColorPalette ColorPaletteSettings::getColorPalette(
        const QString& group, const ColorPalette& defaultPalette) const {
    QList<mixxx::RgbColor> colorList;
    for (const ConfigKey& key : m_pConfig->getKeysWithGroup(group)) {
        mixxx::RgbColor color = mixxx::RgbColor(m_pConfig->getValue<mixxx::RgbColor>(key, mixxx::RgbColor(0)));
        colorList.append(color);
    }

    // If no palette is defined in the settings, we use the default one.
    if (colorList.isEmpty()) {
        return defaultPalette;
    }

    return colorList;
}

void ColorPaletteSettings::setColorPalette(
        const QString& group, const ColorPalette& colorPalette) {
    removePalette(group);

    for (int index = 0; index < colorPalette.m_colorList.count(); ++index) {
        mixxx::RgbColor color = colorPalette.m_colorList[index];
        m_pConfig->setValue<mixxx::RgbColor>(keyForIndex(group, index), color);
    }
}

void ColorPaletteSettings::removePalette(const QString& group) {
    for (const ConfigKey& key : m_pConfig->getKeysWithGroup(group)) {
        m_pConfig->remove(key);
    }
}

ColorPalette ColorPaletteSettings::getHotcueColorPalette() const {
    return getColorPalette(hotcueColorPaletteGroup, ColorPalette::mixxxHotcuePalette);
}

void ColorPaletteSettings::setHotcueColorPalette(const ColorPalette& colorPalette) {
    setColorPalette(hotcueColorPaletteGroup, colorPalette);
}
