#include "preferences/colorpalettesettings.h"

#include <QRegularExpression>

#include "util/color/predefinedcolorpalettes.h"

namespace {
const mixxx::RgbColor kColorBlack(0x000000);
const QString kColorPaletteConfigGroup = QStringLiteral("[Config]");
const QString kColorPaletteGroupStart = QStringLiteral("[ColorPalette ");
const QString kColorPaletteGroupEnd = QStringLiteral("]");
const QRegularExpression kColorPaletteGroupNameRegex(QStringLiteral("^\\[ColorPalette (.+)\\]$"));
const QString kColorPaletteHotcueIndicesConfigItem = QStringLiteral("hotcue_indices");
const QString kColorPaletteHotcueIndicesConfigSeparator = QStringLiteral(" ");
const QString kColorPaletteGroup = QStringLiteral("[ColorPalette %1]");
const ConfigKey kHotcueColorPaletteConfigKey(kColorPaletteConfigGroup, QStringLiteral("HotcueColorPalette"));
const ConfigKey kTrackColorPaletteConfigKey(kColorPaletteConfigGroup, QStringLiteral("TrackColorPalette"));
const ConfigKey kKeyColorPaletteConfigKey(
        kColorPaletteConfigGroup, QStringLiteral("KeyColorPalette"));

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
    if (name.isEmpty()) {
        return defaultPalette;
    }

    // If we find a predefined palette with this name, return it
    for (const ColorPalette& palette : mixxx::PredefinedColorPalettes::kPalettes) {
        if (name == palette.getName()) {
            return palette;
        }
    }

    // Read colors from configuration
    const QString group = kColorPaletteGroupStart + name + kColorPaletteGroupEnd;
    QList<mixxx::RgbColor> colorList;
    QList<int> hotcueIndices;
    const QList<ConfigKey> keys = m_pConfig->getKeysWithGroup(group);
    for (const auto& key : keys) {
        if (key.item == kColorPaletteHotcueIndicesConfigItem) {
            const QStringList stringIndices =
                    m_pConfig->getValueString(key).split(
                            kColorPaletteHotcueIndicesConfigSeparator,
                            Qt::SkipEmptyParts);
            for (const auto& stringIndex : stringIndices) {
                bool ok;
                int index = stringIndex.toInt(&ok);
                if (ok && index >= 0) {
                    hotcueIndices << index;
                }
            }
        } else {
            mixxx::RgbColor color = mixxx::RgbColor(m_pConfig->getValue<mixxx::RgbColor>(key, kColorBlack));
            colorList.append(color);
        }
    }

    // If no palette is defined in the settings, we use the default one.
    if (colorList.isEmpty()) {
        return defaultPalette;
    }

    return ColorPalette(name, colorList, hotcueIndices);
}

void ColorPaletteSettings::setColorPalette(const QString& name, const ColorPalette& colorPalette) {
    VERIFY_OR_DEBUG_ASSERT(!name.isEmpty()) {
        qWarning() << "Palette name must not be empty!";
        return;
    }

    for (const ColorPalette& palette : mixxx::PredefinedColorPalettes::kPalettes) {
        if (name == palette.getName()) {
            qDebug() << "Color Palette" << name << "is a built-in palette, not writing to config!";
            return;
        }
    }
    removePalette(name);
    const QString group = kColorPaletteGroupStart + name + kColorPaletteGroupEnd;

    int numDigits = numberOfDecimalDigits(colorPalette.size() - 1);
    for (int index = 0; index < colorPalette.size(); ++index) {
        mixxx::RgbColor color = colorPalette.at(index);
        m_pConfig->setValue<mixxx::RgbColor>(keyForIndex(group, index, numDigits), color);
    }

    QStringList stringIndices;
    const QList<int> intIndices = colorPalette.getIndicesByHotcue();
    for (const int index : intIndices) {
        stringIndices << QString::number(index);
    }
    if (!stringIndices.isEmpty()) {
        m_pConfig->setValue(
                ConfigKey(group, kColorPaletteHotcueIndicesConfigItem),
                stringIndices.join(kColorPaletteHotcueIndicesConfigSeparator));
    }
}

void ColorPaletteSettings::removePalette(const QString& name) {
    const QString group = kColorPaletteGroupStart + name + kColorPaletteGroupEnd;
    const QList<ConfigKey> keys = m_pConfig->getKeysWithGroup(group);
    for (const ConfigKey& key : keys) {
        m_pConfig->remove(key);
    }
}

ColorPalette ColorPaletteSettings::getHotcueColorPalette() const {
    QString name = m_pConfig->getValueString(kHotcueColorPaletteConfigKey);
    return getHotcueColorPalette(name);
}

ColorPalette ColorPaletteSettings::getHotcueColorPalette(
        const QString& name) const {
    return getColorPalette(
            name,
            mixxx::PredefinedColorPalettes::kDefaultHotcueColorPalette);
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

ColorPalette ColorPaletteSettings::getTrackColorPalette(
        const QString& name) const {
    return getColorPalette(
            name,
            mixxx::PredefinedColorPalettes::kDefaultTrackColorPalette);
}

ColorPalette ColorPaletteSettings::getTrackColorPalette() const {
    QString name = m_pConfig->getValueString(kTrackColorPaletteConfigKey);
    return getTrackColorPalette(name);
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

ColorPalette ColorPaletteSettings::getKeyColorPalette(
        const QString& name) const {
    return getColorPalette(
            name,
            mixxx::PredefinedColorPalettes::kDefaultKeyColorPalette);
}

ColorPalette ColorPaletteSettings::getConfigKeyColorPalette() const {
    QString name = m_pConfig->getValueString(kKeyColorPaletteConfigKey);
    return getKeyColorPalette(name);
}

void ColorPaletteSettings::setKeyColorPalette(const ColorPalette& colorPalette) {
    QString name = colorPalette.getName();
    VERIFY_OR_DEBUG_ASSERT(!name.isEmpty()) {
        qWarning() << "Palette name must not be empty!";
        return;
    }
    m_pConfig->setValue(kKeyColorPaletteConfigKey, name);
    setColorPalette(name, colorPalette);
}

QSet<QString> ColorPaletteSettings::getColorPaletteNames() const {
    QSet<QString> names;
    const QSet<QString> groups = m_pConfig->getGroups();
    for (const auto& group : groups) {
        const QRegularExpressionMatch match = kColorPaletteGroupNameRegex.match(group);
        if (match.hasMatch()) {
            names.insert(match.captured(1));
        }
    }
    return names;
}
