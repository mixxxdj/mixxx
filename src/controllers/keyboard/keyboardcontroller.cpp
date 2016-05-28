#include <QKeyEvent>


#include "keyboardcontroller.h"

#include "controllers/defs_controllers.h"


KeyboardController::KeyboardController() : Controller() {
    setDeviceCategory(tr("Keyboard Controller")); // TODO Add translations

    // TODO(Tomasito) If we add multiple keyboard support, this should be a more specific name
    setDeviceName(tr("Keyboard")); // TODO Add translations
}

KeyboardController::~KeyboardController() {
    // TODO(Tomasito) Think of how the keyboard will close..
}

// TODO(Tomasito) Implement this method
// TODO(Tomasito) When this method is implemented properly, do installEventListener([the instance of KeyboardController])
// TODO ...       on each widget where the KeyboardEventFilter is currently installed

bool KeyboardController::eventFilter(QObject*, QEvent* e) {
    if (e->type() == QEvent::FocusOut) {
        // Clear active key list (TODO: create active key list member)
    }

    if (e->type() == QEvent::KeyPress) {
        QKeyEvent* ke = (QKeyEvent *)e;
        int keyId = ke->nativeScanCode();

        qDebug() << "Key pressed: " << ke->key() << "KeyId =" << keyId;
    }

    else if (e->type() == QEvent::KeyRelease) {
        QKeyEvent* ke = (QKeyEvent *)e;
        int keyId = ke->nativeScanCode();

        qDebug() << "Key released: " << ke->key() << "KeyId =" << keyId;
    }

    return false;
}


QString KeyboardController::presetExtension() {
    return KEYBOARD_PRESET_EXTENSION;
}

bool KeyboardController::savePreset(const QString fileName) const {
    // TODO(Tomasito) Create KeyboardControllerPresetFileHandler class and instantiate here "handler"
    // TODO(Tomasito) handler.save() and return whether it saved successfully (return value of save())
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

int KeyboardController::open() {
    // TODO(Tomasito) Check how this is done in the current keyboard implementation
    return 0;
}

int KeyboardController::close() {
    // TODO(Tomasito) Check how this is done in the current keyboard implementation
    return 0;
}

// TODO(Tomasito) Could this method be inlined?
ControllerPreset *KeyboardController::preset() {
    return &m_preset;
}
