#include "controllers/legacycontrollermapping.h"

void LegacyControllerMapping::loadSettings(const QFileInfo& mappingFile,
        UserSettingsPointer pConfig,
        const QString& controllerName) const {
    QString controllerPath =
            mappingFile.absoluteFilePath()
                    .replace(pConfig->getSettingsPath(),
                            CONTROLLER_SETTINGS_SETTING_PATH_SUBST)
                    .replace(pConfig->getResourcePath(),
                            CONTROLLER_SETTINGS_RESOURCE_PATH_SUBST);

    QString controllerKey = QString(CONTROLLER_SETTINGS_PREFERENCE_GROUP_KEY)
                                    .arg(controllerName, controllerPath);

    auto availableSettings = getSettings();
    QList<ConfigKey> definedSettings = pConfig->getKeysWithGroup(controllerKey);

    QList<QString> availableSettingKeys;
    for (const auto& pSetting : qAsConst(availableSettings)) {
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

void LegacyControllerMapping::saveSettings(const QFileInfo& mappingFile,
        UserSettingsPointer pConfig,
        const QString& controllerName) const {
    QString controllerPath =
            mappingFile.absoluteFilePath()
                    .replace(pConfig->getSettingsPath(), CONTROLLER_SETTINGS_SETTING_PATH_SUBST)
                    .replace(pConfig->getResourcePath(), CONTROLLER_SETTINGS_RESOURCE_PATH_SUBST);
    QString controllerKey = QString(CONTROLLER_SETTINGS_PREFERENCE_GROUP_KEY)
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
