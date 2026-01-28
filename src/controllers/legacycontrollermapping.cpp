#include "controllers/legacycontrollermapping.h"

#include <QFileInfo>

namespace {
const QString kControllerSettingsPreferenceGroupKey = QStringLiteral("[ControllerSettings_%1_%2]");
const QString kControllerSettingsSettingPathSubst = QStringLiteral("%SETTING_PATH");
const QString kControllerSettingsResourcePathSubst = QStringLiteral("%RESOURCE_PATH");
} // anonymous namespace

void LegacyControllerMapping::loadSettings(UserSettingsPointer pConfig,
        const QString& controllerName) const {
    auto mappingFile = QFileInfo(m_filePath);
    if (!mappingFile.exists()) {
        // This happens when we start the MIDI Learning Wizard with a new (empty)
        // mapping, or when the mapping file has been removed manually by the user.
        return;
    }
    QString controllerPath =
            mappingFile.absoluteFilePath()
                    .replace(pConfig->getSettingsPath(),
                            kControllerSettingsSettingPathSubst)
                    .replace(pConfig->getResourcePath(),
                            kControllerSettingsResourcePathSubst);

    QString controllerKey = QString(kControllerSettingsPreferenceGroupKey)
                                    .arg(controllerName, controllerPath);

    auto availableSettings = getSettings();
    const QList<ConfigKey> definedSettings = pConfig->getKeysWithGroup(controllerKey);

    QList<QString> availableSettingKeys;
    for (const auto& pSetting : std::as_const(availableSettings)) {
        availableSettingKeys.append(pSetting->variableName());
    }

    bool ok;
    for (const auto& key : definedSettings) {
        if (!availableSettingKeys.contains(key.item)) {
            qDebug() << "The setting" << key.item
                     << "does not seem to exist in the mapping" << mappingFile.absoluteFilePath()
                     << ". It may be invalid or may have been removed.";
            pConfig->remove(key);
            continue;
        }
        const auto& pSetting = availableSettings.at(availableSettingKeys.indexOf(key.item));
        QString value = pConfig->getValueString(key);
        if (!pSetting->valid()) {
            qWarning() << "The setting" << pSetting->variableName()
                       << "for the mapping" << mappingFile.absoluteFilePath()
                       << "appears to be invalid. Its saved value won't be restored.";
        }
        pSetting->parse(value, &ok);
        if (!ok || !pSetting->valid()) {
            qWarning() << "The setting" << pSetting->variableName()
                       << "for the mapping" << mappingFile.absoluteFilePath()
                       << "could not be restore. Removing and resetting the setting default value.";
            pConfig->remove(key);
            pSetting->reset();
        }
    }
}

void LegacyControllerMapping::resetSettings() {
    for (auto setting : getSettings()) {
        setting->reset();
    }
}

void LegacyControllerMapping::saveSettings(UserSettingsPointer pConfig,
        const QString& controllerName) const {
    auto mappingFile = QFileInfo(m_filePath);
    DEBUG_ASSERT(mappingFile.exists());
    QString controllerPath =
            mappingFile.absoluteFilePath()
                    .replace(pConfig->getSettingsPath(), kControllerSettingsSettingPathSubst)
                    .replace(pConfig->getResourcePath(), kControllerSettingsResourcePathSubst);
    QString controllerKey = QString(kControllerSettingsPreferenceGroupKey)
                                    .arg(controllerName, controllerPath);
    for (auto setting : getSettings()) {
        if (!setting->isDirty()) {
            continue;
        }
        setting->save();
        if (!setting->valid()) {
            qWarning() << "Setting" << setting->variableName()
                       << "for controller" << controllerName
                       << "is invalid. Its value will not be saved.";
            continue;
        }
        if (setting->isDefault()) {
            pConfig->remove(ConfigKey(controllerKey, setting->variableName()));
        } else {
            pConfig->set(ConfigKey(controllerKey, setting->variableName()), setting->stringify());
        }
    }
}

QDebug operator<<(QDebug dbg, const LegacyControllerMapping& mapping) {
    return dbg << "LegacyControllerMapping<"
                  "bDirty="
               << mapping.m_bDirty << ", productInfo=" << mapping.m_productInfo
               << ", deviceId=" << mapping.m_deviceId
               << ", filePath=" << mapping.m_filePath
               << ", name=" << mapping.m_name << ", author=" << mapping.m_author
               << ", description=" << mapping.m_description
               << ", forumlink=" << mapping.m_forumlink
               << ", manualPage=" << mapping.m_manualPage << ", wikilink"
               << mapping.m_wikilink << ", schemaVersion"
               << mapping.m_schemaVersion << ", mixxxVersion"
               << mapping.m_mixxxVersion.toString()
               << ", deviceDirection=" << mapping.m_deviceDirection;
    return dbg;
}

QDebug operator<<(QDebug dbg, const LegacyControllerMapping::ScriptFileInfo& script) {
    return dbg << "ScriptFileInfo<"
                  ", name="
               << script.name << ", identifier=" << script.identifier
               << ", file=" << script.file << ", builtin=" << script.builtin
               << ">";
}

bool operator==(const LegacyControllerMapping::ScriptFileInfo& e1,
        const LegacyControllerMapping::ScriptFileInfo& e2) {
    return e1.name == e2.name &&
            e1.identifier == e2.identifier &&
            e1.file == e2.file &&
            e1.type == e2.type &&
            e1.builtin == e2.builtin;
}

#ifdef MIXXX_USE_QML
QDebug operator<<(QDebug dbg, const LegacyControllerMapping::QMLModuleInfo& module) {
    return dbg << "QMLModuleInfo<builtin=" << module.builtin
               << ", dirinfo=" << module.dirinfo << ">";
}
QDebug operator<<(QDebug dbg, const LegacyControllerMapping::ScreenInfo& screen) {
    return qDebug() << QStringLiteral("ScreenInfo<>");
    return dbg << "ScreenInfo<" << ", identifier" << screen.identifier
               << ", size" << screen.size << ", target_fps" << screen.target_fps
               << ", msaa" << screen.msaa << ", splash_off"
               << screen.splash_off.count() << ", pixelFormat"
               << screen.pixelFormat << ", endian"
               << (screen.endian ==
                                          LegacyControllerMapping::ScreenInfo::
                                                  ColorEndian::Little
                                  ? "little"
                                  : "big")
               << ", reversedColor" << screen.reversedColor << ", rawData"
               << screen.rawData;
}

bool operator==(const LegacyControllerMapping::QMLModuleInfo& e1,
        const LegacyControllerMapping::QMLModuleInfo& e2) {
    return e1.builtin == e2.builtin && e1.dirinfo == e2.dirinfo;
}

bool operator==(const LegacyControllerMapping::ScreenInfo& e1,
        const LegacyControllerMapping::ScreenInfo& e2) {
    return e1.identifier == e2.identifier &&
            e1.size == e2.size &&
            e1.target_fps == e2.target_fps &&
            e1.msaa == e2.msaa &&
            e1.splash_off == e2.splash_off &&
            e1.pixelFormat == e2.pixelFormat &&
            e1.endian == e2.endian &&
            e1.reversedColor == e2.reversedColor &&
            e1.rawData == e2.rawData;
}
#endif
