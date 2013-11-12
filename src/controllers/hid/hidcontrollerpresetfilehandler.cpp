#include "controllers/hid/hidcontrollerpresetfilehandler.h"

bool HidControllerPresetFileHandler::save(const HidControllerPreset& preset,
                                          const QString deviceName,
                                          const QString fileName) const {
    QDomDocument doc = buildRootWithScripts(preset, deviceName);
    return writeDocument(doc, fileName);
}

ControllerPresetPointer HidControllerPresetFileHandler::load(const QDomElement root,
                                                             const QString deviceName,
                                                             const bool forceLoad) {
    if (root.isNull()) {
        return ControllerPresetPointer();
    }

    QDomElement controller = getControllerNode(root, deviceName, forceLoad);
    if (controller.isNull()) {
        return ControllerPresetPointer();
    }

    HidControllerPreset* preset = new HidControllerPreset();
    parsePresetInfo(root, preset);
    addScriptFilesToPreset(controller, preset);
    return ControllerPresetPointer(preset);
}
