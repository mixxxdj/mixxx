#include "macrocontrol.h"

MacroControl::MacroControl(QString group, UserSettingsPointer pConfig, int number)
        : EngineControl(group, pConfig),
          m_number(number),
          m_controlPattern(QString("macro_%1_%2").arg(number)),
          m_COStatus(getConfigKey("status")),
          m_COActive(getConfigKey("active")),
          m_set(getConfigKey("set")),
          m_clear(getConfigKey("clear")),
          m_activate(getConfigKey("activate")) {
    m_COActive.setReadOnly();
    m_COStatus.setReadOnly();

    m_set.setButtonMode(ControlPushButton::TRIGGER);
    connect(&m_set,
            &ControlObject::valueChanged,
            this,
            &MacroControl::controlSet,
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
    m_macro = pNewTrack ? pNewTrack->getMacros().value(m_number) : Macro();
    if (m_macro.m_state.testFlag(Macro::StateFlag::Enabled)) {
        run();
    } else {
        stop();
    }
}

void MacroControl::process(const double dRate, const double dCurrentSample, const int iBufferSize) {
    Q_UNUSED(dRate);
    if (!isRunning()) {
        return;
    }
    double framePos = dCurrentSample / mixxx::kEngineChannelCount;
    const MacroAction& nextAction = m_macro.m_actions.at(m_iNextAction);
    double nextActionPos = nextAction.position;
    int bufFrames = iBufferSize / 2;
    // the process method is called roughly every iBufferSize samples (double as often if you view frames)
    // so we use double that as tolerance range to be safe
    // it triggers early because the seek will only be processed in the next EngineBuffer process call
    if (framePos > nextActionPos - bufFrames && framePos < nextActionPos + bufFrames) {
        seekExact(nextAction.target * mixxx::kEngineChannelCount);
        m_iNextAction++;
        if (m_iNextAction == m_macro.m_actions.size()) {
            if (m_macro.m_state.testFlag(Macro::StateFlag::Looped)) {
                m_iNextAction = 0;
            } else {
                stop();
            }
        }
    }
}

bool MacroControl::isRunning() const {
    return m_iNextAction < m_macro.m_actions.size();
}

void MacroControl::run() {
    m_iNextAction = 0;
    m_COStatus.forceSet(Status::Running);
}

void MacroControl::stop() {
    m_iNextAction = INT_MAX;
    m_COStatus.forceSet(Status::Recorded);
}

void MacroControl::controlSet() {
}

void MacroControl::controlClear() {
    m_macro.m_actions.clear();
}

void MacroControl::controlActivate() {
}
