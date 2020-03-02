#include "preferences/colorpalettesettings.h"

namespace {
const QString kHotcueColorPaletteGroup = QStringLiteral("[HotcueColorPalette]");
const QString kTrackColorPaletteGroup = QStringLiteral("[TrackColorPalette]");
} // anonymous namespace

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

    return ColorPalette(colorList);
}

void ColorPaletteSettings::setColorPalette(
        const QString& group, const ColorPalette& colorPalette) {
    removePalette(group);

    for (int index = 0; index < colorPalette.size(); ++index) {
        mixxx::RgbColor color = colorPalette.at(index);
        m_pConfig->setValue<mixxx::RgbColor>(keyForIndex(group, index), color);
    }
}

void ColorPaletteSettings::removePalette(const QString& group) {
    for (const ConfigKey& key : m_pConfig->getKeysWithGroup(group)) {
        m_pConfig->remove(key);
    }
}

ColorPalette ColorPaletteSettings::getHotcueColorPalette() const {
    return getColorPalette(kHotcueColorPaletteGroup, ColorPalette::mixxxHotcuePalette);
}

void ColorPaletteSettings::setHotcueColorPalette(const ColorPalette& colorPalette) {
    setColorPalette(kHotcueColorPaletteGroup, colorPalette);
}

ColorPalette ColorPaletteSettings::getTrackColorPalette() const {
    return getColorPalette(kTrackColorPaletteGroup, ColorPalette::mixxxHotcuePalette);
}

void ColorPaletteSettings::setTrackColorPalette(const ColorPalette& colorPalette) {
    setColorPalette(kTrackColorPaletteGroup, colorPalette);
}
