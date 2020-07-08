#include "macromanager.h"

#include "preferences/configobject.h"
#include "util/assert.h"

// TODO(xerus) handle track eject while recording
// TODO(xerus) make recording button blink when state is armed

MacroManager::MacroManager()
        : m_COToggleRecording(ConfigKey(kMacroRecordingKey, "recording_toggle")),
          m_CORecStatus(ConfigKey(kMacroRecordingKey, "recording_status")),
          m_activeChannel(nullptr),
          m_macroRecordingState(MacroState::Disabled),
          m_recordedMacro() {
    qCDebug(macros) << "MacroManager construct";

    connect(&m_COToggleRecording,
            &ControlPushButton::valueChanged,
            this,
            &MacroManager::slotToggleRecording);
}

void MacroManager::notifyCueJump(ChannelHandle& channel, double origin, double target) {
    qCDebug(macros) << "Jump in channel" << channel.handle();
    if (checkOrClaimRecording(channel)) {
        m_recordedMacro.appendJump(origin, target);
        qCDebug(macros) << "Recorded jump in channel" << channel.handle();
    }
}

bool MacroManager::checkOrClaimRecording(ChannelHandle& channel) {
    if (m_activeChannel != nullptr) {
        return m_activeChannel->handle() == channel.handle();
    } else if (claimRecording()) {
        m_activeChannel = &channel;
        qCDebug(macros) << "Claimed recording for channel" << channel.handle();
        return true;
    }
    return false;
}

bool MacroManager::claimRecording() {
    auto armed = MacroState::Armed;
    return m_macroRecordingState.compare_exchange_weak(armed, MacroState::Recording);
}

void MacroManager::startRecording() {
    qCDebug(macros) << "MacroManager recording start";
    m_CORecStatus.set(1);
    m_recordedMacro.clear();
    setState(MacroState::Armed);
}

void MacroManager::stopRecording() {
    qCDebug(macros) << "MacroManager recording stop";
    m_CORecStatus.set(0);
    setState(MacroState::Disabled);
    qCDebug(macros) << "Recorded Macro for channel" << m_activeChannel;
    m_activeChannel = nullptr;
    // TODO(xerus) wait until stopped, use stopping state
    m_recordedMacro.dump();
}

Macro MacroManager::getMacro() {
    return m_recordedMacro;
}

ChannelHandle* MacroManager::getActiveChannel() {
    return m_activeChannel;
}

bool MacroManager::isRecordingActive() {
    return getState() != MacroState::Disabled;
}

MacroState MacroManager::getState() {
    return m_macroRecordingState.load();
}

void MacroManager::setState(MacroState state) {
    m_macroRecordingState.store(state);
}
