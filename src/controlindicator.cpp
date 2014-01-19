#include "controlindicator.h"
#include "controlobjectthread.h"

ControlIndicator::ControlIndicator(ConfigKey key)
        : ControlObject(key, false),
          m_blinkValue(OFF),
          m_nextSwitchTime(0.0) {
    m_pCOTGuiTickTime = new ControlObjectThread("[Master]", "guiTickTime"); // Tick time in audio buffer resolution
    m_pCOTGuiTick50ms = new ControlObjectThread("[Master]", "guiTick50ms");
    connect(m_pCOTGuiTick50ms, SIGNAL(valueChanged(double)),
            this, SLOT(slotGuiTick50ms(double)));
    connect(this, SIGNAL(blinkValueChanged()),
            this, SLOT(slotBlinkValueChanged()));
}

ControlIndicator::~ControlIndicator() {
    delete m_pCOTGuiTickTime;
    delete m_pCOTGuiTick50ms;
}

void ControlIndicator::setBlinkValue(enum BlinkValue bv) {
    if (m_blinkValue != bv) {
        m_blinkValue = bv; // must be set at first, to avoid timer toggle
        emit(blinkValueChanged());
    }
}

void ControlIndicator::slotGuiTick50ms(double cpuTime) {
    if (m_nextSwitchTime <= cpuTime) {
        switch (m_blinkValue) {
        case RATIO1TO1_500MS:
            toggle();
            m_nextSwitchTime = cpuTime + 0.5;
            break;
        case RATIO1TO1_250MS:
            toggle();
            m_nextSwitchTime = cpuTime + 0.25;
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
    double oldValue = get();

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
        toggle();
        m_nextSwitchTime = m_pCOTGuiTickTime->get() + 0.5;
        break;
    case RATIO1TO1_250MS:
        toggle();
        m_nextSwitchTime = m_pCOTGuiTickTime->get() + 0.25;
        break;
    default:
        // nothing to do
        break;
    }
}

void ControlIndicator::toggle() {
    set(get()?0.0:1.0);
}
