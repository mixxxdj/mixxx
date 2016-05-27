#include "keyboardcontroller.h"

#include "controllers/defs_controllers.h"


KeyboardController::KeyboardController() : Controller() {
    setDeviceCategory(tr("Keyboard Controller")); // TODO Add translations
}

KeyboardController::~KeyboardController() {
    // TODO Think of how the keyboard will close..
}


QString KeyboardController::presetExtension() {
    return KEYBOARD_PRESET_EXTENSION;
}

bool KeyboardController::savePreset(const QString fileName) const {
    // TODO Create KeyboardControllerPresetFileHandler class and instantiate here "handler"
    // TODO handler.save() and return whether it saved successfully (return value of save())
    return false;
}

void KeyboardController::visit(const KeyboardControllerPreset *preset) {
    m_preset = *preset;
    emit(presetLoaded(getPreset()));
}

void KeyboardController::visit(const MidiControllerPreset *preset) {
    Q_UNUSED(preset);
    qWarning() << "ERROR: Attempting to load a MidiControllerPreset to a KeyboardController!";
    // TODO(XXX): throw a hissy fit.
}

void KeyboardController::visit(const HidControllerPreset *preset) {
    Q_UNUSED(preset);
    qWarning() << "ERROR: Attempting to load an HidControllerPreset to a KeyboardController!";
    // TODO(XXX): throw a hissy fit.
}

bool KeyboardController::matchPreset(const PresetInfo &preset) {
    // Product info mapping not implemented for Keyboards yet
    Q_UNUSED(preset);
    return false;
}