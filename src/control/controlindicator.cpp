#include "control/controlindicator.h"

#include "control/controlproxy.h"
#include "moc_controlindicator.cpp"

namespace {
const QString kAppGroup = QStringLiteral("[App]");
} // namespace

ControlIndicator::ControlIndicator(const ConfigKey& key)
        : ControlObject(key, false),
          m_blinkValue(OFF),
          m_pCOIndicator250millis(make_parented<ControlProxy>(
                  kAppGroup, QStringLiteral("indicator_250ms"), this)),
          m_pCOIndicator500millis(make_parented<ControlProxy>(
                  kAppGroup, QStringLiteral("indicator_500ms"), this)) {
    connect(this,
            &ControlIndicator::blinkValueChanged,
            this,
            &ControlIndicator::slotBlinkValueChanged);
}

ControlIndicator::~ControlIndicator() {
}

void ControlIndicator::setBlinkValue(enum BlinkValue newBlinkValue) {
    if (m_blinkValue != newBlinkValue) {
        // Disconnect old signals
        switch (m_blinkValue) {
        case RATIO1TO1_250MS:
            m_pCOIndicator250millis->disconnect(this);
            break;
        case RATIO1TO1_500MS:
            m_pCOIndicator500millis->disconnect(this);
            break;
        default:
            // Nothing to do
            break;
        }

        m_blinkValue = newBlinkValue;
        switch (m_blinkValue) {
        case RATIO1TO1_250MS:
            m_pCOIndicator250millis->connectValueChanged(this, &ControlIndicator::slotValueChanged);
            break;
        case RATIO1TO1_500MS:
            m_pCOIndicator500millis->connectValueChanged(this, &ControlIndicator::slotValueChanged);
            break;
        default:
            // Nothing to do
            break;
        }
        emit blinkValueChanged();
    }
}

void ControlIndicator::slotBlinkValueChanged() {
    const bool oldValue = toBool();

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
    case RATIO1TO1_250MS:
        set(m_pCOIndicator250millis->get());
        break;
    case RATIO1TO1_500MS:
        set(m_pCOIndicator500millis->get());
        break;
    default:
        // nothing to do
        break;
    }
}
