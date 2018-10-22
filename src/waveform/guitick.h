#ifndef GUITICK_H
#define GUITICK_H

#include <QObject>

#include "control/controlobject.h"
#include "util/duration.h"
#include "util/memory.h"
#include "util/performancetimer.h"

class GuiTick : public QObject {
    Q_OBJECT
  public:
    GuiTick(QObject* pParent = NULL);
    ~GuiTick() = default;
    void process();

  private:
    std::unique_ptr<ControlObject> m_pCOGuiTickTime;
    std::unique_ptr<ControlObject> m_pCOGuiTick20ms;
    std::unique_ptr<ControlObject> m_pCOGuiTick50ms;
    PerformanceTimer m_cpuTimer;
    mixxx::Duration m_lastUpdateTime20ms;
    mixxx::Duration m_lastUpdateTime50ms;
    mixxx::Duration m_cpuTimeLastTick;
};

#endif // GUITICK_H
