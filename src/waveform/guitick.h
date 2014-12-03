#ifndef GUITICK_H
#define GUITICK_H

#include <QObject>

#include "mixxx.h"
#include "util/performancetimer.h"

class ControlObject;
class QTimer;

class GuiTick : public QObject {
    Q_OBJECT
  public:
	GuiTick(QObject* pParent, MixxxMainWindow* mixxx);
    ~GuiTick();
    void process();
    static double cpuTime();

  private:
    ControlObject* m_pCOGuiTickTime;
    ControlObject* m_pCOGuiTick50ms;

    PerformanceTimer m_cpuTimer;

	bool tablet;
	bool tabletMode;
	bool lastTabletMode;
	double m_lastUpdateTime;
	double m_lastUpdateTime2;
	MixxxMainWindow* m_mixxx;
    static double m_cpuTime; // Stream Time in seconds
};

#endif // GUITICK_H
