#include <QKeyEvent>

#include "control/controlobject.h"
#include "controllers/defs_controllers.h"
#include "controllers/keyboard/keyboardcontroller.h"
#include "keyboardcontrollerpresetfilehandler.h"


KeyboardController::KeyboardController(KeyboardEventFilter* pKbdEventFilter) :
        Controller(),
        m_pKbdEventFilter(pKbdEventFilter) {
    setDeviceCategory(tr("Keyboard Controller")); // TODO(XXX) Add translations

    // TODO(Tomasito) If we add multiple keyboard support, this should be a more specific name
    setDeviceName(tr("Keyboard")); // TODO Add translations

    connect(m_pKbdEventFilter, SIGNAL(controlKeySeqPressed(ConfigKey)),
            this, SLOT(onKeySeqPressed(ConfigKey)));

    connect(this, SIGNAL(keyboardControllerPresetLoaded(KeyboardControllerPresetPointer)),
            m_pKbdEventFilter, SLOT(slotSetKeyboardMapping(KeyboardControllerPresetPointer)));

    connect(m_pKbdEventFilter, SIGNAL(keyboardLayoutChanged(QString)),
            this, SLOT(reloadPreset(QString)));
}

KeyboardController::~KeyboardController() {
    close();
}


QString KeyboardController::presetExtension() {
    return KEYBOARD_PRESET_EXTENSION;
}

bool KeyboardController::savePreset(const QString fileName) const {
    KeyboardControllerPresetFileHandler handler;
    return handler.save(m_preset, getName(), fileName);
}

void KeyboardController::visitKeyboard(const KeyboardControllerPreset* preset) {
    m_preset = *preset;
    emit(presetLoaded(getPreset()));
    emit(keyboardControllerPresetLoaded(getKeyboardPreset()));
} 

void KeyboardController::visitMidi(const MidiControllerPreset* preset) {
    Q_UNUSED(preset);
    qWarning() << "ERROR: Attempting to load a MidiControllerPreset to a KeyboardController!";
    // TODO(XXX): throw a hissy fit.
}

void KeyboardController::visitHid(const HidControllerPreset* preset) {
    Q_UNUSED(preset);
    qWarning() << "ERROR: Attempting to load an HidControllerPreset to a KeyboardController!";
    // TODO(XXX): throw a hissy fit.
}

bool KeyboardController::matchPreset(const PresetInfo& preset) {
    // Product info mapping not implemented for Keyboards yet
    Q_UNUSED(preset);
    return false;
}

int KeyboardController::open() {
    setOpen(true);
    emit enabled(true);
    return 0;
}

int KeyboardController::close() {
    setOpen(false);
    emit enabled(false);
    return 0;
}

void KeyboardController::onKeySeqPressed(ConfigKey configKey) {
    if (!isOpen()) return;

    ControlObject* control = ControlObject::getControl(configKey);

    // Since setting the value might cause us to go down
    // a route that would eventually clear the active
    // key list, do that last.
    control->setValueFromMidi(MIDI_NOTE_ON, 1);
}

void KeyboardController::reloadPreset(QString layout) {
    // Reload preset
    m_preset.translate(layout);
    emit keyboardControllerPresetLoaded(getKeyboardPreset());
}