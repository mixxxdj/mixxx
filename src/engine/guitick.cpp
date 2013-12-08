#include "engine/guitick.h"
#include "controlobject.h"


// static
double GuiTick::m_streamTime = 0.0;
double GuiTick::m_cpuTime = 0.0;

GuiTick::GuiTick(QObject* pParent)
        : QObject(pParent),
          m_lastUpdateTime(0.0) {
     m_pCOStreamTime = new ControlObject(ConfigKey("[Master]", "streamTime"));
     m_pCOCpuTime = new ControlObject(ConfigKey("[Master]", "cpuTime"));
     m_pCOGuiTick50ms = new ControlObject(ConfigKey("[Master]", "guiTick50ms"));
     m_cpuTimer.start();
}

GuiTick::~GuiTick() {
    delete m_pCOStreamTime;
    delete m_pCOGuiTick50ms;
}

void GuiTick::process() {
    qint64 elapsedNs = m_cpuTimer.restart();
    double elapsedS = elapsedNs / 1000000000.0;
    m_cpuTime += elapsedS;
    m_pCOCpuTime->set(m_cpuTime);

    if (m_lastUpdateTime + 0.05 < m_cpuTime) {
        m_lastUpdateTime = m_cpuTime;
        m_pCOGuiTick50ms->set(m_streamTime);
    }

    m_pCOStreamTime->set(m_streamTime);
}

// static
void GuiTick::setStreamTime(double streamTime) {
    m_streamTime = streamTime;
}

// static
double GuiTick::streamTime() {
    return m_streamTime;
}

// static
double GuiTick::cpuTime() {
    return m_cpuTime;
}


