#include "controlindicator.h"
#include "controlobjectthread.h"

ControlIndicator::ControlIndicator(ConfigKey key)
        : ControlObject(key, false),
          m_blinkValue(OFF),
          m_on(FALSE),
          m_nextSwitchTime(0.0) {
    m_pCOTGuiTick50ms = new ControlObjectThread("[Master]", "guiTick50ms");
    connect(m_pCOTGuiTick50ms, SIGNAL(valueChanged(double)),
            this, SLOT(slotGuiTick50ms(double)));

}

ControlIndicator::~ControlIndicator() {
    delete m_pCOTGuiTick50ms;
}

void ControlIndicator::setBlinkValue(enum BlinkValue bv) {
    m_blinkValue = bv;
    switch (m_blinkValue) {
    case OFF:
        set(0.0);
        break;
    case ON:
        set(1.0);
        break;
    case RATIO1TO1_500MS: // fall through
    case RATIO1TO1_250MS: // fall through
    default:
        // nothing to do
        break;
    }
}

void ControlIndicator::slotGuiTick50ms(double streamTime) {
    if (m_nextSwitchTime <= streamTime) {
        switch (m_blinkValue) {
        case RATIO1TO1_500MS:
            toggle();
            m_nextSwitchTime += 0.5;
            break;
        case RATIO1TO1_250MS:
            toggle();
            m_nextSwitchTime += 0.25;
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
    set(m_on?0.0:1.0);
    m_on = !m_on;
}
