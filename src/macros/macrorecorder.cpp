#include "macros/macrorecorder.h"

#include <QtConcurrentRun>

#include "control/controlproxy.h"
#include "preferences/configobject.h"

namespace {
constexpr unsigned int kStartCheckInterval = 300;
constexpr size_t kMaxMacroSize = 1000;
const QString kConfigGroup = QStringLiteral("[MacroRecording]");
} // namespace

MacroRecorder::MacroRecorder()
        : m_COToggleRecording(ConfigKey(kConfigGroup, "record")),
          m_CORecStatus(ConfigKey(kConfigGroup, "status")),
          m_activeChannel(nullptr),
          m_pStartRecordingTimer(this),
          m_recordedActions(kMaxMacroSize) {
    qCDebug(macroLoggingCategory) << "MacroRecorder construct";

    m_COToggleRecording.setButtonMode(ControlPushButton::TOGGLE);
    m_CORecStatus.setReadOnly();
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
        m_recordedActions.try_emplace(sourceFramePos, destFramePos);
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
    m_CORecStatus.forceSet(Status::Recording);
}

void MacroRecorder::startRecording() {
    qCDebug(macroLoggingCategory) << "MacroRecorder recording armed";
    m_CORecStatus.forceSet(Status::Armed);
    m_pStartRecordingTimer.start(kStartCheckInterval);
}

void MacroRecorder::stopRecording() {
    qCDebug(macroLoggingCategory) << "MacroRecorder recording stop";
    m_pStartRecordingTimer.stop();
    m_CORecStatus.forceSet(Status::Disabled);

    ChannelHandle* channel = m_activeChannel.exchange(nullptr);
    if (channel == nullptr) {
        return;
    }

    emit saveMacroFromChannel(fetchRecordedActions(), *channel);
}

size_t MacroRecorder::getRecordingSize() const {
    return m_recordedActions.size();
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

QList<MacroAction> MacroRecorder::fetchRecordedActions() {
    QList<MacroAction> actions;
    while (MacroAction* action = m_recordedActions.front()) {
        actions.append(*action);
        m_recordedActions.pop();
    }
    return actions;
}

void MacroRecorder::notifyTrackChange(ChannelHandle* channel, TrackPointer track) {
    if (m_activeChannel.compare_exchange_strong(channel, nullptr)) {
        m_pStartRecordingTimer.stop();
        m_CORecStatus.forceSet(Status::Disabled);

        emit saveMacro(fetchRecordedActions(), track);
    }
}
