#include <QTimer>

#include "guitick.h"
#include "controlobject.h"

GuiTick::GuiTick(QObject* pParent)
        : QObject(pParent) {
     m_pCOGuiTickTime = new ControlObject(ConfigKey("[Master]", "guiTickTime"));
     m_pCOGuiTick50ms = new ControlObject(ConfigKey("[Master]", "guiTick50ms"));
     m_cpuTimer.start();
}

GuiTick::~GuiTick() {
    delete m_pCOGuiTickTime;
    delete m_pCOGuiTick50ms;
}

// this is called from the VSyncThread
// with the configured waveform frame rate
void GuiTick::process() {
    m_cpuTimeLastTick += m_cpuTimer.restart();
    double cpuTimeLastTickSeconds = m_cpuTimeLastTick.toDoubleSeconds();
    m_pCOGuiTickTime->set(cpuTimeLastTickSeconds);

    if (m_cpuTimeLastTick - m_lastUpdateTime >= mixxx::Duration::fromMillis(50)) {
        m_lastUpdateTime = m_cpuTimeLastTick;
        m_pCOGuiTick50ms->set(cpuTimeLastTickSeconds);
    }
}
