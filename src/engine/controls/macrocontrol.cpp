#include "macrocontrol.h"

// TODO(xerus) move recording here and delete macromanager

MacroControl::MacroControl(QString group, UserSettingsPointer pConfig, int number)
        : EngineControl(group, pConfig),
          m_iNextAction(0),
          m_number(number),
          m_controlPattern(QString("macro_%1_%2").arg(number)),
          m_COStatus(getConfigKey("status")),
          m_COActive(getConfigKey("active")),
          m_toggle(getConfigKey("toggle")),
          m_clear(getConfigKey("clear")),
          m_activate(getConfigKey("activate")) {
    m_COActive.setReadOnly();
    m_COStatus.setReadOnly();
    m_COStatus.forceSet(Status::NoTrack);

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

ConfigKey MacroControl::getConfigKey(QString name) {
    return ConfigKey(m_group, m_controlPattern.arg(name));
}

void MacroControl::trackLoaded(TrackPointer pNewTrack) {
    m_pMacro = pNewTrack ? pNewTrack->getMacros().value(m_number) : nullptr;
    if (m_pMacro) {
        if (m_pMacro->isEmpty()) {
            m_COStatus.forceSet(Status::Empty);
        } else {
            if (m_pMacro->isEnabled()) {
                play();
            } else {
                stop();
            }
        }
    } else {
        m_COStatus.forceSet(Status::NoTrack);
    }
}

void MacroControl::process(const double dRate, const double dCurrentSample, const int iBufferSize) {
    Q_UNUSED(dRate);
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
                m_COStatus.forceSet(Status::PlaybackStopped);
            }
        }
    }
}

MacroControl::Status MacroControl::getStatus() const {
    return Status(m_COStatus.get());
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
    m_COStatus.forceSet(Status::Playing);
}

void MacroControl::stop() {
    DEBUG_ASSERT(m_pMacro);
    m_iNextAction = INT_MAX;
    m_COStatus.forceSet(Status::Recorded);
}

void MacroControl::controlRecord() {
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
