#include "controlindicator.h"
#include "controlobjectthread.h"

ControlIndicator::ControlIndicator(ConfigKey key)
        : ControlObject(key, false),
          m_blinkValue(OFF),
          m_nextSwitchTime(0.0) {
    m_pCOTCpuTime = new ControlObjectThread("[Master]", "cpuTime");
    m_pCOTGuiTick50ms = new ControlObjectThread("[Master]", "guiTick50ms");
    connect(m_pCOTGuiTick50ms, SIGNAL(valueChanged(double)),
            this, SLOT(slotGuiTick50ms(double)));

}

ControlIndicator::~ControlIndicator() {
    delete m_pCOTCpuTime;
    delete m_pCOTGuiTick50ms;
}

void ControlIndicator::setBlinkValue(enum BlinkValue bv) {
    if (m_blinkValue != bv) {
        m_blinkValue = bv; // must be set at first, to avoid timer toggle
        switch (m_blinkValue) {
        case OFF:
            set(0.0);
            break;
        case ON:
            set(1.0);
            break;
        case RATIO1TO1_500MS:
            toggle();
            m_nextSwitchTime = m_pCOTCpuTime->get() + 0.5;
            break;
        case RATIO1TO1_250MS:
            toggle();
            m_nextSwitchTime = m_pCOTCpuTime->get() + 0.25;
            break;
        default:
            // nothing to do
            break;
        }
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

void ControlIndicator::toggle() {
    set(get()?0.0:1.0);
}
