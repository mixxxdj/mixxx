#include <QTimer>

#include "guitick.h"
#include "controlobject.h"


// static
double GuiTick::m_cpuTimeLastTick = 0.0;

GuiTick::GuiTick(QObject* pParent)
        : QObject(pParent),
          m_lastUpdateTime(0.0) {
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
    qint64 elapsedNs = m_cpuTimer.restart();
    double elapsedS = elapsedNs / 1000000000.0;
    m_cpuTimeLastTick += elapsedS;
    m_pCOGuiTickTime->set(m_cpuTimeLastTick);

    if (m_lastUpdateTime + 0.05 < m_cpuTimeLastTick) {
        m_lastUpdateTime = m_cpuTimeLastTick;
        m_pCOGuiTick50ms->set(m_cpuTimeLastTick);
    }
}

// static
double GuiTick::cpuTimeLastTick() {
     return m_cpuTimeLastTick;
}
