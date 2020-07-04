#include "macromanager.h"

// TODO(xerus) handle track eject while recording
// TODO(xerus) write tests
// TODO(xerus) make recording button blink when state is armed

MacroManager::MacroManager()
        : m_COToggleRecording(ControlPushButton(ConfigKey(kMacroRecordingKey, "recording_toggle"))),
          m_CORecStatus(ControlObject(ConfigKey(kMacroRecordingKey, "recording_status"))),
          m_activeChannel(-1),
          m_macroRecordingState(MacroState::Disabled),
          m_recordedMacro(Macro()) {
    qCDebug(macros) << "MacroManager construct";

    connect(&m_COToggleRecording,
            &ControlPushButton::valueChanged,
            this,
            &MacroManager::slotToggleRecording);
}

bool MacroManager::notifyCueJump(int channel, double origin, double target) {
    if (claimRecording(channel)) {
        m_recordedMacro.appendJump(origin, target);
        return true;
    }
    return false;
}

bool MacroManager::claimRecording(int channel) {
    if (m_activeChannel == channel) {
        return true;
    } else {
        auto armed = MacroState::Armed;
        if (m_macroRecordingState.compare_exchange_weak(armed, MacroState::Recording)) {
            m_activeChannel = channel;
            return true;
        }
    }
    return false;
}

void MacroManager::startRecording() {
    qCDebug(macros) << "MacroManager recording start";
    m_CORecStatus.set(1);
    m_recordedMacro.clear();
    m_macroRecordingState.store(MacroState::Armed);
}

void MacroManager::stopRecording() {
    qCDebug(macros) << "MacroManager recording stop";
    m_CORecStatus.set(0);
    m_macroRecordingState.store(MacroState::Disabled);
    qCDebug(macros) << "Recorded Macro for channel" << m_activeChannel;
    m_activeChannel = -1;
    // TODO(xerus) wait until stopped, use stopping state
    m_recordedMacro.dump();
}

bool MacroManager::isRecordingActive() {
    return m_macroRecordingState.load() != MacroState::Disabled;
}
