#include "macrorecorder.h"

#include <QtConcurrentRun>

#include "control/controlproxy.h"
#include "preferences/configobject.h"

// TODO(xerus) handle track eject while recording

MacroRecorder::MacroRecorder()
        : m_COToggleRecording(ConfigKey(kMacroRecordingKey, "recording_toggle")),
          m_CORecStatus(ConfigKey(kMacroRecordingKey, "recording_status")),
          m_activeChannel(nullptr),
          m_macroRecordingState(MacroState::Disabled),
          m_recordedMacro() {
    qCDebug(macros) << "MacroRecorder construct";

    connect(&m_COToggleRecording,
            &ControlPushButton::valueChanged,
            this,
            &MacroRecorder::slotToggleRecording);
}

void MacroRecorder::notifyCueJump(ChannelHandle& channel, double origin, double target) {
    qCDebug(macros) << "Jump in channel" << channel.handle();
    if (checkOrClaimRecording(channel)) {
        m_recordedMacro.appendJump(origin, target);
        qCDebug(macros) << "Recorded jump in channel" << channel.handle();
        setState(MacroState::Armed);
    }
}

bool MacroRecorder::checkOrClaimRecording(ChannelHandle& channel) {
    if (m_activeChannel != nullptr) {
        return m_activeChannel->handle() == channel.handle() && claimRecording();
    } else if (claimRecording()) {
        m_activeChannel = &channel;
        qCDebug(macros) << "Claimed recording for channel" << channel.handle();
        return true;
    }
    return false;
}

bool MacroRecorder::claimRecording() {
    auto armed = MacroState::Armed;
    return m_macroRecordingState.compare_exchange_weak(armed, MacroState::Recording);
}

void pollRecordingStart(MacroRecorder* pMacroRecorder) {
    while (pMacroRecorder->getActiveChannel() == nullptr) {
        QThread::msleep(300);
        if (pMacroRecorder->getState() == MacroState::Disabled) {
            return;
        }
    }
    // TODO(xerus) add test
    ControlProxy(ConfigKey(kMacroRecordingKey, "recording_status")).set(2);
}

void MacroRecorder::startRecording() {
    qCDebug(macros) << "MacroRecorder recording start";
    m_CORecStatus.set(1);
    m_recordedMacro.clear();
    setState(MacroState::Armed);
    QtConcurrent::run(pollRecordingStart, this);
}

void MacroRecorder::stopRecording() {
    qCDebug(macros) << "MacroRecorder recording stop";
    m_CORecStatus.set(0);
    auto armed = MacroState::Armed;
    // TODO(xerus) add concurrency test
    while (!m_macroRecordingState.compare_exchange_weak(armed, MacroState::Disabled))
        QThread::yieldCurrentThread();
    if (m_activeChannel == nullptr)
        return;
    auto channel = m_activeChannel;
    m_activeChannel = nullptr;
    qCDebug(macros) << "Recorded Macro for channel" << channel->handle();
    m_recordedMacro.dump();
    emit saveMacro(*channel, m_recordedMacro);
}

Macro MacroRecorder::getMacro() {
    return m_recordedMacro;
}

ChannelHandle* MacroRecorder::getActiveChannel() {
    return m_activeChannel;
}

bool MacroRecorder::isRecordingActive() {
    return getState() != MacroState::Disabled;
}

MacroState MacroRecorder::getState() {
    return m_macroRecordingState.load();
}

void MacroRecorder::setState(MacroState state) {
    m_macroRecordingState.store(state);
}
