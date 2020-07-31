#include "macrorecorder.h"

#include <QtConcurrentRun>

#include "control/controlproxy.h"
#include "preferences/configobject.h"

// TODO(xerus) handle track eject while recording

namespace {
constexpr uint kMaxMacroSize = 1000;
const QString kConfigGroup = QStringLiteral("[MacroRecording]");
}

MacroRecorder::MacroRecorder()
        : m_COToggleRecording(ConfigKey(kConfigGroup, "recording_toggle")),
          m_CORecStatus(ConfigKey(kConfigGroup, "recording_status")),
          m_activeChannel(nullptr),
          m_pStartRecordingTimer(this),
          m_pRecordedActions(kMaxMacroSize) {
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
    if (isRecordingActive() && checkOrClaimRecording(channel)) {
        m_pRecordedActions.emplace(sourceFramePos, destFramePos);
        qCDebug(macroLoggingCategory) << "Recorded jump in channel" << channel->handle();
    }
}

bool MacroRecorder::checkOrClaimRecording(ChannelHandle* channel) {
    ChannelHandle* value = nullptr;
    return m_activeChannel.compare_exchange_strong(value, channel) ||
            value->handle() == channel->handle();
}

void MacroRecorder::pollRecordingStart() {
    qCDebug(macroLoggingCategory) << "Polling for recording start";
    if (getActiveChannel() == nullptr) {
        return;
    }
    m_pStartRecordingTimer.stop();
    m_CORecStatus.set(Status::Recording);
}

void MacroRecorder::startRecording() {
    qCDebug(macroLoggingCategory) << "MacroRecorder recording armed";
    m_CORecStatus.set(Status::Armed);
    m_pStartRecordingTimer.start(300);
}

void MacroRecorder::stopRecording() {
    qCDebug(macroLoggingCategory) << "MacroRecorder recording stop";
    m_pStartRecordingTimer.stop();
    m_CORecStatus.set(Status::Disabled);

    ChannelHandle* channel = m_activeChannel.exchange(nullptr);
    if (channel == nullptr) {
        return;
    }

    QVector<MacroAction> actions;
    while (MacroAction* action = m_pRecordedActions.front()) {
        m_pRecordedActions.pop();
        actions.append(*action);
    }
    emit saveMacro(*channel, actions);
}

const MacroAction MacroRecorder::getRecordedAction() {
    return *m_pRecordedActions.front();
}

size_t MacroRecorder::getRecordingSize() const {
    return m_pRecordedActions.size();
}

const ChannelHandle* MacroRecorder::getActiveChannel() const {
    return m_activeChannel.load();
}

MacroRecorder::Status MacroRecorder::getStatus() const {
    return Status(m_CORecStatus.get());
}

bool MacroRecorder::isRecordingActive() const {
    return getStatus() > 0;
}