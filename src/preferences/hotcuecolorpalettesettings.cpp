#include "preferences/hotcuecolorpalettesettings.h"
#include "control/controlobject.h"

namespace {
constexpr QRgb kOffPaletteDefault = 0xf36100; // library icons orange;
} // anonymous namespace

const QString HotcueColorPaletteSettings::sGroup = "[HotcueColorPalette]";

ColorPalette HotcueColorPaletteSettings::getHotcueColorPalette() const {
    QList<QRgb> colorList;
    for (const ConfigKey& key : m_pConfig->getKeysWithGroup(sGroup)) {
        QColor color = m_pConfig->getValue<QColor>(key);
        if (color.isValid()) {
            QRgb rgb = color.rgb() & RGB_MASK; // The mask is required for some reasons. A Qt bug?
            DEBUG_ASSERT(rgb <= 0xFFFFFF);
            colorList.append(rgb);
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

QRgb HotcueColorPaletteSettings::getDefaultColor(int hotcue, bool* keepDefault) const {
    if (keepDefault) {
        *keepDefault = false;
    }


    ConfigKey autoHotcueColorsKey("[Controls]", "HotcueDefaultColorIndex");
    //  QRgb(0xf36100) library icons orange
    int index = m_pConfig->getValue(autoHotcueColorsKey, -3);

    if (index == -1) {
        if (keepDefault) {
            *keepDefault = true;
        }
        return QRgb(0xf36100); // library icons orange
    }

    if (index == -2) {
        if (keepDefault) {
            *keepDefault = true;
        }
        index = hotcue;
    }

    if (index < 0) {
        // -3 or invalid hotcue return skin default
        double dRgb = ControlObject::get(ConfigKey("[Skin]","hotcue_default_color"));
        if (dRgb <= 0xFFFFFF && dRgb >= 0) {
            return QRgb(dRgb);
        }
        if (keepDefault) {
            *keepDefault = true;
        }
        return offPaletteDefaultColor();
    }

    auto hotcueColorPalette = getHotcueColorPalette();
    auto colors = hotcueColorPalette.m_colorList;
    return colors.at(index % colors.count());
}

QRgb HotcueColorPaletteSettings::offPaletteDefaultColor() const {
    return kOffPaletteDefault;
}

