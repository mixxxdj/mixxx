#include "engine/guitick.h"
#include "controlobject.h"


// static
double GuiTick::m_streamTime = 0.0;

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
    m_pCOStreamTime->set(m_streamTime);
    if (m_lastUpdateTime > m_streamTime || m_lastUpdateTime + 0.05 < m_streamTime) {
        m_lastUpdateTime = m_streamTime;
        m_pCOGuiTick50ms->set(m_streamTime);
    }
}

// static
void GuiTick::setStreamTime(double streamTime) {
    m_streamTime = streamTime;
}

// static
double GuiTick::streamTime() {
    return m_streamTime;
}


