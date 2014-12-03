#include <QTimer>

#include "guitick.h"
#include "controlobject.h"
#include "mixxx.h"
#include <Windows.h>

#define SM_CONVERTIBLESLATEMODE   0x2003


// static
double GuiTick::m_cpuTime = 0.0;

GuiTick::GuiTick(QObject* pParent, MixxxMainWindow* mixxx)
        : QObject(pParent),
		  m_mixxx(mixxx),
          m_lastUpdateTime(0.0),
		  m_lastUpdateTime2(0.0),
		  tabletMode(false),
		  lastTabletMode(false),
		  tablet(false){
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
    m_cpuTime += elapsedS;
    m_pCOGuiTickTime->set(m_cpuTime);

	if (m_lastUpdateTime2 + 3.00 < m_cpuTime)
	{
		tabletMode = (GetSystemMetrics(SM_CONVERTIBLESLATEMODE) == 0);
		if (tabletMode != lastTabletMode)
		{
			lastTabletMode = tabletMode;
			m_mixxx->skinChange(tabletMode);
		}
		lastTabletMode = tabletMode;
		m_lastUpdateTime2 = m_cpuTime;
	}

    if (m_lastUpdateTime + 0.05 < m_cpuTime) {
        m_lastUpdateTime = m_cpuTime;
        m_pCOGuiTick50ms->set(m_cpuTime);
    }
}

// static
double GuiTick::cpuTime() {
    return m_cpuTime;
}
