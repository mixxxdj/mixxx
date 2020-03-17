#include "preferences/colorpalettesettings.h"

namespace {
const mixxx::RgbColor kColorBlack(0x000000);
const QString kColorPaletteConfigGroup = QStringLiteral("[Config]");
const QString kColorPaletteGroupStart = QStringLiteral("[ColorPalette ");
const QString kColorPaletteGroupEnd = QStringLiteral("]");
const QRegExp kColorPaletteGroupNameRegex(QStringLiteral("^\\[ColorPalette (.+)\\]$"));
const ConfigKey kHotcueColorPaletteConfigKey(kColorPaletteConfigGroup, QStringLiteral("HotcueColorPalette"));
const ConfigKey kTrackColorPaletteConfigKey(kColorPaletteConfigGroup, QStringLiteral("TrackColorPalette"));

int numberOfDecimalDigits(int number) {
    int numDigits = 1;
    while (number /= 10) {
        numDigits++;
    }
    return numDigits;
}

ConfigKey keyForIndex(const QString& group, int index, int numDigits) {
    return ConfigKey(group, QString::number(index).rightJustified(numDigits, '0'));
}

} // anonymous namespace

ColorPalette ColorPaletteSettings::getColorPalette(
        const QString& name, const ColorPalette& defaultPalette) const {
    QList<mixxx::RgbColor> colorList;

    const QString group = kColorPaletteGroupStart + name + kColorPaletteGroupEnd;
    for (const ConfigKey& key : m_pConfig->getKeysWithGroup(group)) {
        mixxx::RgbColor color = mixxx::RgbColor(m_pConfig->getValue<mixxx::RgbColor>(key, kColorBlack));
        colorList.append(color);
    }

    // If no palette is defined in the settings, we use the default one.
    if (colorList.isEmpty()) {
        return defaultPalette;
    }

    return ColorPalette(name, colorList);
}

void ColorPaletteSettings::setColorPalette(const QString& name, const ColorPalette& colorPalette) {
    VERIFY_OR_DEBUG_ASSERT(!name.isEmpty()) {
        qWarning() << "Palette name must not be empty!";
        return;
    }

    removePalette(name);
    const QString group = kColorPaletteGroupStart + name + kColorPaletteGroupEnd;

    int numDigits = numberOfDecimalDigits(colorPalette.size() - 1);
    for (int index = 0; index < colorPalette.size(); ++index) {
        mixxx::RgbColor color = colorPalette.at(index);
        m_pConfig->setValue<mixxx::RgbColor>(keyForIndex(group, index, numDigits), color);
    }
}

void ColorPaletteSettings::removePalette(const QString& name) {
    const QString group = kColorPaletteGroupStart + name + kColorPaletteGroupEnd;
    for (const ConfigKey& key : m_pConfig->getKeysWithGroup(group)) {
        m_pConfig->remove(key);
    }
}

ColorPalette ColorPaletteSettings::getHotcueColorPalette() const {
    QString name = m_pConfig->getValueString(kHotcueColorPaletteConfigKey);
    if (name.isEmpty()) {
        return ColorPalette::mixxxHotcuePalette;
    }
    return getColorPalette(name, ColorPalette::mixxxHotcuePalette);
}

void ColorPaletteSettings::setHotcueColorPalette(const ColorPalette& colorPalette) {
    QString name = colorPalette.getName();
    VERIFY_OR_DEBUG_ASSERT(!name.isEmpty()) {
        qWarning() << "Palette name must not be empty!";
        return;
    }
    m_pConfig->setValue(kHotcueColorPaletteConfigKey, name);
    setColorPalette(name, colorPalette);
}

ColorPalette ColorPaletteSettings::getTrackColorPalette() const {
    QString name = m_pConfig->getValueString(kTrackColorPaletteConfigKey);
    if (name.isEmpty()) {
        return ColorPalette::mixxxHotcuePalette;
    }
    return getColorPalette(name, ColorPalette::mixxxHotcuePalette);
}

void ColorPaletteSettings::setTrackColorPalette(const ColorPalette& colorPalette) {
    QString name = colorPalette.getName();
    VERIFY_OR_DEBUG_ASSERT(!name.isEmpty()) {
        qWarning() << "Palette name must not be empty!";
        return;
    }
    m_pConfig->setValue(kTrackColorPaletteConfigKey, name);
    setColorPalette(name, colorPalette);
}

QSet<QString> ColorPaletteSettings::getColorPaletteNames() const {
    QSet<QString> names;
    for (const QString& group : m_pConfig->getGroups()) {
        int pos = kColorPaletteGroupNameRegex.indexIn(group);
        if (pos > -1) {
            names.insert(kColorPaletteGroupNameRegex.cap(1));
        }
    }
    return names;
}
