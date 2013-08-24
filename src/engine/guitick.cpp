#include "engine/guitick.h"
#include "controlobject.h"


// static
double GuiTick::m_streamtime = 0.0;

GuiTick::GuiTick(QObject* pParent)
        : QObject(pParent),
          m_lastUpdateTime(0.0) {
     m_pCOStreamTime = new ControlObject(ConfigKey("[Master]", "streamTime"));
     m_pCOGuiTick50ms = new ControlObject(ConfigKey("[Master]", "guiTick50ms"));
}

GuiTick::~GuiTick() {
    delete m_pCOStreamTime;
    delete m_pCOGuiTick50ms;
}

void GuiTick::process() {
    m_pCOStreamTime->set(m_streamtime);
    if (m_lastUpdateTime > m_streamtime || m_lastUpdateTime + 0.05 < m_streamtime) {
        m_pCOGuiTick50ms->set(m_streamtime);
    }
}

// static
void GuiTick::setStreamTime(double streamTime) {
    m_streamtime = streamTime;
}

// static
double GuiTick::streamTime() {
    return m_streamtime;
}


