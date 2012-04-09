#include "controllers/hidcontrollerpresetfilehandler.h"
#include "controllers/hidcontrollerpreset.h"

bool HidControllerPresetFileHandler::save(
    const HidControllerPreset& preset,
    const QString deviceName, const QString fileName) const {
    QDomDocument doc = buildRootWithScripts(preset, deviceName);
    return writeDocument(root, fileName);
}

ControllerPreset* HidControllerPresetFileHandler::load(
    const QDomElement root, const QString deviceName, const bool forceLoad) {
    HidControllerPreset* preset = new HidControllerPreset();
    addScriptFilesToMapping(root, deviceName, forceLoad, preset);
    return preset;
}
