#include "control/controlindicator.h"

#include "control/controlproxy.h"
#include "moc_controlindicator.cpp"
#include "util/math.h"

ControlIndicator::ControlIndicator(const ConfigKey& key)
        : ControlObject(key, false),
          m_blinkValue(OFF),
          m_nextSwitchTime(0.0) {
    // Tick time in audio buffer resolution
    m_pCOTGuiTickTime = new ControlProxy("[Master]", "guiTickTime", this);
    m_pCOTGuiTick50ms = new ControlProxy("[Master]", "guiTick50ms", this);
    m_pCOTGuiTick50ms->connectValueChanged(this, &ControlIndicator::slotGuiTick50ms);
    connect(this,
            &ControlIndicator::blinkValueChanged,
            this,
            &ControlIndicator::slotBlinkValueChanged);
}

ControlIndicator::~ControlIndicator() {
}

void ControlIndicator::setBlinkValue(enum BlinkValue bv) {
    if (m_blinkValue != bv) {
        m_blinkValue = bv; // must be set at first, to avoid timer toggle
        emit blinkValueChanged();
    }
}

void ControlIndicator::slotGuiTick50ms(double cpuTime) {
    if (m_nextSwitchTime <= cpuTime) {
        switch (m_blinkValue) {
        case RATIO1TO1_500MS:
            toggle(0.5);
            break;
        case RATIO1TO1_250MS:
            toggle(0.25);
            break;
        case OFF: // fall through
        case ON: // fall through
        default:
            // nothing to do
            break;
        }
    }
}

void ControlIndicator::slotBlinkValueChanged() {
    bool oldValue = toBool();

    switch (m_blinkValue) {
    case OFF:
        if (oldValue) {
            set(0.0);
        }
        break;
    case ON:
        if (!oldValue) {
            set(1.0);
        }
        break;
    case RATIO1TO1_500MS:
        toggle(0.5);
        break;
    case RATIO1TO1_250MS:
        toggle(0.25);
        break;
    default:
        // nothing to do
        break;
    }
}

void ControlIndicator::toggle(double duration) {
    double tickTime = m_pCOTGuiTickTime->get();
    double toggles = floor(tickTime / duration);
    bool phase = fmod(toggles, 2) >= 1;
    bool val = toBool();
    if(val != phase) {
        // Out of phase, wait until we are in phase
        m_nextSwitchTime = (toggles + 2) * duration;
    } else {
        m_nextSwitchTime = (toggles + 1) * duration;
    }
    set(val ? 0.0 : 1.0);
}
