#include "waveform/guitick.h"

#include "control/controlobject.h"

namespace {
const QString kAppGroup = QStringLiteral("[App]");
const QString kLegacyGroup = QStringLiteral("[Master]");
} // namespace

GuiTick::GuiTick() {
    m_pCOGuiTickTime = std::make_unique<ControlObject>(
            ConfigKey(kAppGroup, QStringLiteral("gui_tick_full_period_s")));
    m_pCOGuiTickTime->addAlias(ConfigKey(kLegacyGroup, QStringLiteral("guiTickTime")));
    m_pCOGuiTick50ms = std::make_unique<ControlObject>(
            ConfigKey(kAppGroup, QStringLiteral("gui_tick_50ms_period_s")));
    m_pCOGuiTick50ms->addAlias(ConfigKey(kLegacyGroup, QStringLiteral("guiTick50ms")));
    m_cpuTimer.start();
}

// this is called from WaveformWidgetFactory::render in the main thread with the
// configured waveform frame rate
void GuiTick::process() {
    m_cpuTimeLastTick += m_cpuTimer.restart();
    double cpuTimeLastTickSeconds = m_cpuTimeLastTick.toDoubleSeconds();
    m_pCOGuiTickTime->set(cpuTimeLastTickSeconds);

    if (m_cpuTimeLastTick - m_lastUpdateTime >= mixxx::Duration::fromMillis(50)) {
        m_lastUpdateTime = m_cpuTimeLastTick;
        m_pCOGuiTick50ms->set(cpuTimeLastTickSeconds);
    }
}
