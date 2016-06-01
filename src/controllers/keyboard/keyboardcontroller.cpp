#include <QKeyEvent>


#include "keyboardcontroller.h"

#include "controllers/defs_controllers.h"


KeyboardController::KeyboardController(KeyboardEventFilter* pKbdEventFilter) :
        Controller(),
        m_pKbdEventFilter(pKbdEventFilter) {
    setDeviceCategory(tr("Keyboard Controller")); // TODO Add translations

    // TODO(Tomasito) If we add multiple keyboard support, this should be a more specific name
    setDeviceName(tr("Keyboard")); // TODO Add translations

    connect(m_pKbdEventFilter, SIGNAL(keySeqPressed(QKeySequence)),
            this, SLOT(onKeySeqPressed(QKeySequence)));
}

KeyboardController::~KeyboardController() {
    // TODO(Tomasito) Think of how the keyboard will close..
}


QString KeyboardController::presetExtension() {
    return KEYBOARD_PRESET_EXTENSION;
}

bool KeyboardController::savePreset(const QString fileName) const {
    Q_UNUSED(fileName);
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

void KeyboardController::onKeySeqPressed(QKeySequence ks) {
    qDebug() << "KeyboardController::onKeySeqPressed() " << ks.toString();
}


