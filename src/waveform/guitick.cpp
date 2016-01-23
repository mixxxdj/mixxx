#include <QTimer>

#include "guitick.h"
#include "controlobject.h"

GuiTick::GuiTick(QObject* pParent)
        : QObject(pParent),
          m_lastUpdateTimeSeconds(0.0),
          m_cpuTimeLastTickSeconds(0.0) {
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
    mixxx::Duration elapsed = m_cpuTimer.restart();
    m_cpuTimeLastTickSeconds += elapsed.toDoubleSeconds();
    m_pCOGuiTickTime->set(m_cpuTimeLastTickSeconds);

    if (m_lastUpdateTimeSeconds + 0.05 < m_cpuTimeLastTickSeconds) {
        m_lastUpdateTimeSeconds = m_cpuTimeLastTickSeconds;
        m_pCOGuiTick50ms->set(m_cpuTimeLastTickSeconds);
    }
}
