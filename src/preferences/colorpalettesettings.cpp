#include "preferences/colorpalettesettings.h"

ColorPaletteSettings::ColorPaletteSettings(QString group, UserSettingsPointer pConfig)
        : m_configGroup(group),
          m_palette(ColorPalette::mixxxPalette),
          m_pConfig(pConfig) {
    loadFromConfig();
}

ColorPaletteSettings::~ColorPaletteSettings() {
    save();
}

void ColorPaletteSettings::loadFromConfig() {
    VERIFY_OR_DEBUG_ASSERT(!m_configGroup.isEmpty()) {
        return;
    }

    QList<QColor> colorList;
    for (const ConfigKey& key : m_pConfig->getKeysWithGroup(m_configGroup)) {
        QColor color = m_pConfig->getValue<QColor>(key);
        if (color.isValid()) {
            colorList.append(color);
        }
    }

    m_palette = ColorPalette(colorList);
}

void ColorPaletteSettings::save() {
    for (int index = 0; index < m_palette.m_colorList.count(); ++index) {
        QColor color = m_palette.m_colorList[index];
        m_pConfig->setValue<QColor>(keyForIndex(index), color);
    }
}
