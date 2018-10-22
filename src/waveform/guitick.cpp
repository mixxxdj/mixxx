#include <QTimer>

#include "waveform/guitick.h"
#include "control/controlobject.h"

GuiTick::GuiTick(QObject* pParent)
        : QObject(pParent) {
    m_pCOGuiTickTime = std::make_unique<ControlObject>(ConfigKey("[Master]", "guiTickTime"));
    m_pCOGuiTick20ms = std::make_unique<ControlObject>(ConfigKey("[Master]", "guiTick20ms"));
    m_pCOGuiTick50ms = std::make_unique<ControlObject>(ConfigKey("[Master]", "guiTick50ms"));
    m_cpuTimer.start();
}

// this is called from the VSyncThread
// with the configured waveform frame rate
void GuiTick::process() {
    m_cpuTimeLastTick += m_cpuTimer.restart();
    double cpuTimeLastTickSeconds = m_cpuTimeLastTick.toDoubleSeconds();
    m_pCOGuiTickTime->set(cpuTimeLastTickSeconds);

    if (m_cpuTimeLastTick - m_lastUpdateTime20ms >= mixxx::Duration::fromMillis(20)) {
        m_lastUpdateTime20ms = m_cpuTimeLastTick;
        m_pCOGuiTick20ms->set(cpuTimeLastTickSeconds);
    }
    if (m_cpuTimeLastTick - m_lastUpdateTime50ms >= mixxx::Duration::fromMillis(50)) {
        m_lastUpdateTime50ms = m_cpuTimeLastTick;
        m_pCOGuiTick50ms->set(cpuTimeLastTickSeconds);
    }
}
