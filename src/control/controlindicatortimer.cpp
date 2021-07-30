#include "control/controlindicatortimer.h"

#include "control/controlobject.h"
#include "moc_controlindicatortimer.cpp"

namespace mixxx {

ControlIndicatorTimer::ControlIndicatorTimer(QObject* pParent)
        : QObject(pParent),
          m_pCOIndicator250millis(std::make_unique<ControlObject>(
                  ConfigKey("[Master]", "indicator_250millis"))),
          m_pCOIndicator500millis(std::make_unique<ControlObject>(
                  ConfigKey("[Master]", "indicator_500millis"))) {
    m_pCOIndicator250millis->setReadOnly();
    m_pCOIndicator500millis->setReadOnly();
    connect(&m_timer, &QTimer::timeout, this, &ControlIndicatorTimer::slotTimeout);
    m_timer.start(250);
}

void ControlIndicatorTimer::slotTimeout() {
    // The timeout uses an interval of 250ms, so the 250ms indicator is always updated.
    const bool indicator250millisEnabled = m_pCOIndicator250millis->toBool();
    m_pCOIndicator250millis->forceSet(indicator250millisEnabled ? 0.0 : 1.0);

    // The 500ms indicator is only updated on every second call to the function.
    if (indicator250millisEnabled) {
        m_pCOIndicator500millis->forceSet(m_pCOIndicator500millis->toBool() ? 0.0 : 1.0);
    }
}

} // namespace mixxx
