#include "control/controlindicatortimer.h"

#include "control/controlobject.h"
#include "moc_controlindicatortimer.cpp"
#include "util/math.h"

namespace {
const QString kAppGroup = QStringLiteral("[App]");
const QString kLegacyGroup = QStringLiteral("[Master]");
} // namespace

namespace mixxx {

ControlIndicatorTimer::ControlIndicatorTimer(QObject* pParent)
        : QObject(pParent),
          m_pCOIndicator250millis(std::make_unique<ControlObject>(
                  ConfigKey(kAppGroup, QStringLiteral("indicator_250ms")))),
          m_pCOIndicator500millis(std::make_unique<ControlObject>(
                  ConfigKey(kAppGroup, QStringLiteral("indicator_500ms")))),
          m_nextSwitchTime(0.0),
          m_pCPGuiTick50ms(nullptr) {
    m_pCOIndicator250millis->setReadOnly();
    m_pCOIndicator250millis->addAlias(
            ConfigKey(kLegacyGroup, QStringLiteral("indicator_250millis")));
    m_pCOIndicator500millis->setReadOnly();
    m_pCOIndicator500millis->addAlias(
            ConfigKey(kLegacyGroup, QStringLiteral("indicator_500millis")));
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

// TODO: Everything below this comment only added for compatibility with the
// legacy waveform vsync thread. It should be removed when the legacy skin
// system is dropped.

void ControlIndicatorTimer::setLegacyVsyncEnabled(bool enabled) {
    const bool isLegacyVsyncEnabled = (m_pCPGuiTick50ms != nullptr);
    if (isLegacyVsyncEnabled == enabled) {
        return;
    }

    if (enabled) {
        m_timer.stop();
        m_pCPGuiTick50ms = std::make_unique<ControlProxy>(
                ConfigKey(kAppGroup, QStringLiteral("gui_tick_50ms_period_s")));
        m_pCPGuiTick50ms->connectValueChanged(this, &ControlIndicatorTimer::slotGuiTick50ms);
    } else {
        m_pCPGuiTick50ms->disconnect(this);
        m_pCPGuiTick50ms.reset();
        m_timer.start(250);
    }
}

void ControlIndicatorTimer::slotGuiTick50ms(double cpuTime) {
    if (m_nextSwitchTime > cpuTime) {
        return;
    }

    constexpr double duration = 0.25;
    const double tickTime = ControlObject::get(
            ConfigKey(kAppGroup, QStringLiteral("gui_tick_full_period_s")));
    const double toggles = floor(tickTime / duration);
    m_nextSwitchTime = (toggles + 1) * duration;
    slotTimeout();
}

} // namespace mixxx
