#include "keyboardcontrollerpresetfilehandler.h"


KeyboardControllerPresetFileHandler::KeyboardControllerPresetFileHandler() {

}

KeyboardControllerPresetFileHandler::~KeyboardControllerPresetFileHandler() {

}

ControllerPresetPointer KeyboardControllerPresetFileHandler::load(const QDomElement root, const QString deviceName) {
    if (root.isNull()) {
        return ControllerPresetPointer();
    }

    QDomElement controller = getControllerNode(root, deviceName);
    if (controller.isNull()) {
        return ControllerPresetPointer();
    }

    KeyboardControllerPreset* preset = new KeyboardControllerPreset();

    // Superclass handles parsing <info> tag and script files
    parsePresetInfo(root, preset);
    addScriptFilesToPreset(controller, preset);

    return ControllerPresetPointer(preset);
}