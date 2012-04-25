#include "controllers/hid/hidcontrollerpresetfilehandler.h"

bool HidControllerPresetFileHandler::save(const HidControllerPreset& preset,
                                          const QString deviceName,
                                          const QString fileName) const {
    QDomDocument doc = buildRootWithScripts(preset, deviceName);
    return writeDocument(doc, fileName);
}

ControllerPreset* HidControllerPresetFileHandler::load(const QDomElement root,
                                                       const QString deviceName,
                                                       const bool forceLoad) {
    HidControllerPreset* preset = new HidControllerPreset();
    addScriptFilesToPreset(root, deviceName, forceLoad, preset);
    return preset;
}
