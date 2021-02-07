#include <QTimer>

#include "waveform/guitick.h"
#include "control/controlobject.h"

GuiTick::GuiTick() {
    m_pCOGuiTickTime = std::make_unique<ControlObject>(ConfigKey("[Master]", "guiTickTime"));
    m_pCOGuiTick50ms = std::make_unique<ControlObject>(ConfigKey("[Master]", "guiTick50ms"));
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
