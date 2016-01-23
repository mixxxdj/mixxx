#ifndef GUITICK_H
#define GUITICK_H

#include <QObject>

#include "util/performancetimer.h"

class ControlObject;
class QTimer;

class GuiTick : public QObject {
    Q_OBJECT
  public:
    GuiTick(QObject* pParent = NULL);
    ~GuiTick();
    void process();

  private:
    ControlObject* m_pCOGuiTickTime;
    ControlObject* m_pCOGuiTick50ms;
    PerformanceTimer m_cpuTimer;
    double m_lastUpdateTimeSeconds;
    double m_cpuTimeLastTickSeconds;
};

#endif // GUITICK_H
