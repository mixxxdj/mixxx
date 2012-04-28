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
    if (root.isNull()) {
        return preset;
    }

    QDomElement controller = getControllerNode(root, deviceName, forceLoad);
    if (controller.isNull()) {
        return preset;
    }

    addScriptFilesToPreset(controller, preset);
    return preset;
}
