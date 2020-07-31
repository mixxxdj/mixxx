#include "macrorecorder.h"

#include <QtConcurrentRun>

#include "control/controlproxy.h"
#include "preferences/configobject.h"

// TODO(xerus) handle track eject while recording

namespace {
const QString kConfigGroup = QStringLiteral("[MacroRecording]");
}

MacroRecorder::MacroRecorder()
        : m_COToggleRecording(ConfigKey(kConfigGroup, "recording_toggle")),
          m_CORecStatus(ConfigKey(kConfigGroup, "recording_status")),
          m_activeChannel(nullptr),
          m_macroRecordingState(State::Disabled),
          m_pStartRecordingTimer(this),
          m_recordedMacro() {
    qCDebug(macroLoggingCategory) << "MacroRecorder construct";

    connect(&m_COToggleRecording,
            &ControlPushButton::valueChanged,
            this,
            &MacroRecorder::slotToggleRecording);
    connect(&m_pStartRecordingTimer,
            &QTimer::timeout,
            this,
            &MacroRecorder::pollRecordingStart);
}

void MacroRecorder::notifyCueJump(
        ChannelHandle* channel, double sourceFramePos, double destFramePos) {
    qCDebug(macroLoggingCategory) << "Jump in channel" << channel->handle();
    if (checkOrClaimRecording(channel)) {
        m_recordedMacro.appendJump(sourceFramePos, destFramePos);
        qCDebug(macroLoggingCategory) << "Recorded jump in channel" << channel->handle();
        setState(State::Armed);
    }
}

bool MacroRecorder::checkOrClaimRecording(ChannelHandle* channel) {
    if (m_activeChannel != nullptr) {
        return m_activeChannel->handle() == channel->handle() && claimRecording();
    } else if (claimRecording()) {
        m_activeChannel = channel;
        qCDebug(macroLoggingCategory) << "Claimed recording for channel" << channel->handle();
        return true;
    }
    return false;
}

bool MacroRecorder::claimRecording() {
    auto armed = State::Armed;
    return m_macroRecordingState.compare_exchange_weak(armed, State::Recording);
}

void MacroRecorder::pollRecordingStart() {
    qCDebug(macroLoggingCategory) << "Polling for recording start";
    if (getActiveChannel() == nullptr) {
        return;
    }
    if (getState() != State::Disabled) {
        m_CORecStatus.set(Status::Recording);
    }
    m_pStartRecordingTimer.stop();
}

void MacroRecorder::startRecording() {
    qCDebug(macroLoggingCategory) << "MacroRecorder recording armed";
    m_CORecStatus.set(Status::Armed);
    m_recordedMacro.clear();
    setState(State::Armed);
    m_pStartRecordingTimer.start(300);
}

void MacroRecorder::stopRecording() {
    qCDebug(macroLoggingCategory) << "MacroRecorder recording stop";
    auto armed = State::Armed;
    while (!m_macroRecordingState.compare_exchange_strong(armed, State::Disabled)) {
        QThread::yieldCurrentThread();
        if (getState() == State::Disabled) {
            return;
        }
        armed = State::Armed;
    }
    m_CORecStatus.set(Status::Disabled);
    if (m_activeChannel == nullptr) {
        return;
    }
    auto channel = m_activeChannel;
    m_activeChannel = nullptr;
    emit saveMacro(*channel, m_recordedMacro);
}

Macro MacroRecorder::getMacro() const {
    return m_recordedMacro;
}

ChannelHandle* MacroRecorder::getActiveChannel() const {
    return m_activeChannel;
}

bool MacroRecorder::isRecordingActive() const {
    return getState() != State::Disabled;
}

MacroRecorder::State MacroRecorder::getState() const {
    return m_macroRecordingState.load();
}

void MacroRecorder::setState(State state) {
    m_macroRecordingState.store(state);
}
