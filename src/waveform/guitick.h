#ifndef GUITICK_H
#define GUITICK_H

#include <QObject>

#include "util/duration.h"
#include "util/performancetimer.h"

class ControlObject;

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
    mixxx::Duration m_lastUpdateTime;
    mixxx::Duration m_cpuTimeLastTick;
};

#endif // GUITICK_H
