#include "controllers/hidcontrollerpresetfilehandler.h"
#include "controllers/hidcontrollerpreset.h"

ControllerPreset* HidControllerPresetFileHandler::load(
    const QDomElement root, const QString deviceName, const bool forceLoad) {
    HidControllerPreset* preset = new HidControllerPreset();
    addScriptFilesToMapping(root, deviceName, forceLoad, preset);
    return preset;
}
