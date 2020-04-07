#include "controllers/hid/hidcontrollerpresetfilehandler.h"

bool HidControllerPresetFileHandler::save(const HidControllerPreset& preset,
                                          const QString deviceName,
                                          const QString fileName) const {
    QDomDocument doc = buildRootWithScripts(preset, deviceName);
    return writeDocument(doc, fileName);
}

ControllerPresetPointer HidControllerPresetFileHandler::load(const QDomElement root,
        const QString filePath,
        const QString deviceName,
        const QDir& systemPresetsPath) {
    if (root.isNull()) {
        return ControllerPresetPointer();
    }

    QDomElement controller = getControllerNode(root, deviceName);
    if (controller.isNull()) {
        return ControllerPresetPointer();
    }

    HidControllerPreset* preset = new HidControllerPreset();
    preset->setFilePath(filePath);
    parsePresetInfo(root, preset);
    addScriptFilesToPreset(controller, preset, systemPresetsPath);
    return ControllerPresetPointer(preset);
}
