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
