#include "macrorecorder.h"

#include <QtConcurrentRun>

#include "control/controlproxy.h"
#include "preferences/configobject.h"

// TODO(xerus) handle track eject while recording

const QString MacroRecorder::kControlsGroup = QStringLiteral("[MacroRecording]");

MacroRecorder::MacroRecorder()
        : m_COToggleRecording(ConfigKey(kControlsGroup, "recording_toggle")),
          m_CORecStatus(ConfigKey(kControlsGroup, "recording_status")),
          m_activeChannel(nullptr),
          m_macroRecordingState(MacroRecordingState::Disabled),
          m_pStartRecordingTimer(this),
          m_recordedMacro() {
    qCDebug(macros) << "MacroRecorder construct";

    connect(&m_COToggleRecording,
            &ControlPushButton::valueChanged,
            this,
            &MacroRecorder::slotToggleRecording);
    connect(&m_pStartRecordingTimer,
            &QTimer::timeout,
            this,
            &MacroRecorder::pollRecordingStart);
}

void MacroRecorder::notifyCueJump(ChannelHandle& channel, double origin, double target) {
    qCDebug(macros) << "Jump in channel" << channel.handle();
    if (checkOrClaimRecording(channel)) {
        m_recordedMacro.appendJump(origin, target);
        qCDebug(macros) << "Recorded jump in channel" << channel.handle();
        setState(MacroRecordingState::Armed);
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
    auto armed = MacroRecordingState::Armed;
    return m_macroRecordingState.compare_exchange_weak(armed, MacroRecordingState::Recording);
}

void MacroRecorder::pollRecordingStart() {
    qCDebug(macros) << "Polling for recording start";
    if (getState() != MacroRecordingState::Disabled) {
        if (getActiveChannel() == nullptr) {
            return;
        } else {
            ControlProxy(ConfigKey(kControlsGroup, "recording_status")).set(2);
        }
    }
    m_pStartRecordingTimer.stop();
}

void MacroRecorder::startRecording() {
    qCDebug(macros) << "MacroRecorder recording armed";
    m_CORecStatus.set(1);
    m_recordedMacro.clear();
    setState(MacroRecordingState::Armed);
    m_pStartRecordingTimer.start(300);
}

void MacroRecorder::stopRecording() {
    qCDebug(macros) << "MacroRecorder recording stop";
    m_CORecStatus.set(0);
    auto armed = MacroRecordingState::Armed;
    // TODO(xerus) add concurrency test
    while (!m_macroRecordingState.compare_exchange_weak(armed, MacroRecordingState::Disabled)) {
        QThread::yieldCurrentThread();
    }
    if (m_activeChannel == nullptr) {
        return;
    }
    auto channel = m_activeChannel;
    m_activeChannel = nullptr;
    qCDebug(macros) << "Recorded Macro for channel" << channel->handle();
    m_recordedMacro.dump();
    emit saveMacro(*channel, m_recordedMacro);
}

Macro MacroRecorder::getMacro() const {
    return m_recordedMacro;
}

ChannelHandle* MacroRecorder::getActiveChannel() const {
    return m_activeChannel;
}

bool MacroRecorder::isRecordingActive() const {
    return getState() != MacroRecordingState::Disabled;
}

MacroRecordingState MacroRecorder::getState() const {
    return m_macroRecordingState.load();
}

void MacroRecorder::setState(MacroRecordingState state) {
    m_macroRecordingState.store(state);
}
