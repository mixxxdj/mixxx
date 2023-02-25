#include "controllers/legacycontrollermapping.h"

void LegacyControllerMapping::restoreSettings(const QFileInfo& mappingFile,
        UserSettingsPointer pConfig,
        const QString& controllerName) {
    QString controllerKey = QString(CONTROLLER_SETTINGS_PREFERENCE_GROUP_KEY)
                                    .arg(controllerName)
                                    .arg(mappingFile.absoluteFilePath());
    for (auto setting : getSettings()) {
        bool ok;
        QString value = pConfig->getValueString(ConfigKey(controllerKey, setting->variableName()));
        setting->parse(value, &ok);
        if (!ok) {
            qWarning() << "The setting" << setting->variableName()
                       << "for the mapping" << mappingFile.absoluteFilePath()
                       << "could not be restore. Removing";
            pConfig->remove(ConfigKey(controllerKey, setting->variableName()));
        }
    }
    // TODO (acolombier): If there are other settings that aren't defined
    // anymore, they should be removed.
}

void LegacyControllerMapping::saveSettings(const QFileInfo& mappingFile,
        UserSettingsPointer pConfig,
        const QString& controllerName) {
    QString controllerKey = QString(CONTROLLER_SETTINGS_PREFERENCE_GROUP_KEY)
                                    .arg(controllerName)
                                    .arg(mappingFile.absoluteFilePath());
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
