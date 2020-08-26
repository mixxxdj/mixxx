#include "macrocontrol.h"

namespace {
constexpr uint kRecordingTimerInterval = 100;
constexpr size_t kRecordingQueueSize = kRecordingTimerInterval / 10;
} // namespace

ConfigKey MacroControl::getConfigKey(QString name) {
    return ConfigKey(m_group, m_controlPattern.arg(name));
}
MacroControl::MacroControl(QString group, UserSettingsPointer pConfig, int number)
        : EngineControl(group, pConfig),
          m_number(number),
          m_controlPattern(QString("macro_%1_%2").arg(number)),
          m_bJumpPending(false),
          m_recordedActions(kRecordingQueueSize),
          m_iNextAction(0),
          m_COStatus(getConfigKey("status")),
          m_COIndicator(getConfigKey("indicator")),
          m_record(getConfigKey("record")),
          m_toggle(getConfigKey("toggle")),
          m_clear(getConfigKey("clear")),
          m_activate(getConfigKey("activate")) {
    m_COIndicator.setReadOnly();
    m_COStatus.setReadOnly();
    setStatus(Status::NoTrack);

    m_record.setButtonMode(ControlPushButton::TRIGGER);
    connect(&m_record,
            &ControlObject::valueChanged,
            this,
            &MacroControl::controlRecord,
            Qt::DirectConnection);
    m_toggle.setButtonMode(ControlPushButton::TRIGGER);
    connect(&m_toggle,
            &ControlObject::valueChanged,
            this,
            &MacroControl::controlToggle,
            Qt::DirectConnection);
    m_clear.setButtonMode(ControlPushButton::TRIGGER);
    connect(&m_clear,
            &ControlObject::valueChanged,
            this,
            &MacroControl::controlClear,
            Qt::DirectConnection);
    connect(&m_activate,
            &ControlObject::valueChanged,
            this,
            &MacroControl::controlActivate,
            Qt::DirectConnection);
}

void MacroControl::process(const double dRate, const double dCurrentSample, const int iBufferSize) {
    Q_UNUSED(dRate);
    m_bJumpPending = false;
    if (!isPlaying()) {
        return;
    }
    double framePos = dCurrentSample / mixxx::kEngineChannelCount;
    const MacroAction& nextAction = m_pMacro->getActions().at(m_iNextAction);
    double nextActionPos = nextAction.position;
    int bufFrames = iBufferSize / 2;
    // the process method is called roughly every iBufferSize samples (double as often if you view frames)
    // so we use double that as tolerance range to be safe
    // it triggers early because the seek will only be processed in the next EngineBuffer process call
    if (framePos > nextActionPos - bufFrames && framePos < nextActionPos + bufFrames) {
        seekExact(nextAction.target * mixxx::kEngineChannelCount);
        m_iNextAction++;
        if (m_iNextAction == m_pMacro->size()) {
            if (m_pMacro->isLooped()) {
                m_iNextAction = 0;
            } else {
                setStatus(Status::PlaybackStopped);
            }
        }
    }
}

void MacroControl::trackLoaded(TrackPointer pNewTrack) {
    if (isRecording()) {
        stopRecording();
    }
    m_pMacro = pNewTrack ? pNewTrack->getMacros().value(m_number) : nullptr;
    if (m_pMacro) {
        if (m_pMacro->isEmpty()) {
            setStatus(Status::Empty);
        } else {
            if (m_pMacro->isEnabled()) {
                play();
            } else {
                stop();
            }
        }
    } else {
        setStatus(Status::NoTrack);
    }
}

void MacroControl::notifySeek(double dNewPlaypos) {
    if (!m_bJumpPending) {
        return;
    }
    m_bJumpPending = false;
    if (getStatus() == Status::Armed) {
        setStatus(Status::Recording);
    }
    if (getStatus() != Status::Recording) {
        return;
    }
    double sourceFramePos = getSampleOfTrack().current / mixxx::kEngineChannelCount;
    double destFramePos = dNewPlaypos / mixxx::kEngineChannelCount;
    m_recordedActions.try_emplace(sourceFramePos, destFramePos);
}

void MacroControl::slotJumpQueued() {
    m_bJumpPending = true;
}

MacroControl::Status MacroControl::getStatus() const {
    return Status(m_COStatus.get());
}

void MacroControl::setStatus(Status status) {
    m_COStatus.forceSet(status);
    if (status > Status::Empty) {
        // TODO(xerus) add blinking for Status::Recording & Status::Playing
        m_COIndicator.forceSet(status > Status::Empty ? 1 : 0);
    }
}

MacroPtr MacroControl::getMacro() const {
    return m_pMacro;
}

bool MacroControl::isRecording() const {
    return getStatus() == Status::Armed || getStatus() == Status::Recording;
}

bool MacroControl::isPlaying() const {
    DEBUG_ASSERT(m_iNextAction >= 0);
    return m_pMacro && m_iNextAction < m_pMacro->size();
}

void MacroControl::play() {
    DEBUG_ASSERT(m_pMacro);
    m_iNextAction = 0;
    setStatus(Status::Playing);
}

void MacroControl::stop() {
    DEBUG_ASSERT(m_pMacro);
    m_iNextAction = INT_MAX;
    setStatus(Status::Recorded);
}

void MacroControl::updateRecording() {
    VERIFY_OR_DEBUG_ASSERT(isRecording()) {
        return;
    }
    if (m_recordedActions.empty()) {
        return;
    }
    if (getStatus() == Status::Armed) {
        setStatus(Status::Recording);
    }
    while (MacroAction* action = m_recordedActions.front()) {
        m_pMacro->addAction(*action);
        m_recordedActions.pop();
    }
}

void MacroControl::stopRecording() {
    VERIFY_OR_DEBUG_ASSERT(isRecording()) {
        return;
    }
    m_updateRecordingTimer.stop();
    updateRecording();
    if (getStatus() == Status::Armed) {
        setStatus(Status::Empty);
    } else {
        setStatus(Status::Recorded);
        play();
    }
}

void MacroControl::controlRecord() {
    if (getStatus() == Status::NoTrack) {
        return;
    }
    if (!isRecording()) {
        setStatus(Status::Armed);
        m_updateRecordingTimer.start(kRecordingTimerInterval);
    } else {
        stopRecording();
    }
}

void MacroControl::controlToggle() {
    if (m_pMacro) {
        m_pMacro->setState(Macro::StateFlag::Enabled, !m_pMacro->isEnabled());
    }
}

void MacroControl::controlClear() {
    if (m_pMacro && !isPlaying()) {
        m_pMacro->clear();
    }
}

void MacroControl::controlActivate() {
    if (!m_pMacro || m_pMacro->isEmpty()) {
        controlRecord();
    } else {
        if (isPlaying()) {
            seekExact(m_pMacro->getActions().first().target);
        } else {
            play();
        }
    }
}
