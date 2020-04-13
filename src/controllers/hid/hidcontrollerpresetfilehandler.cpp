#include "controllers/hid/hidcontrollerpresetfilehandler.h"

bool HidControllerPresetFileHandler::save(const HidControllerPreset& preset,
        const QString& fileName) const {
    QDomDocument doc = buildRootWithScripts(preset);
    return writeDocument(doc, fileName);
}

ControllerPresetPointer HidControllerPresetFileHandler::load(const QDomElement& root,
        const QString& filePath,
        const QDir& systemPresetsPath) {
    if (root.isNull()) {
        return ControllerPresetPointer();
    }

    QDomElement controller = getControllerNode(root);
    if (controller.isNull()) {
        return ControllerPresetPointer();
    }

    HidControllerPreset* preset = new HidControllerPreset();
    preset->setFilePath(filePath);
    parsePresetInfo(root, preset);
    addScriptFilesToPreset(controller, preset, systemPresetsPath);
    return ControllerPresetPointer(preset);
}
