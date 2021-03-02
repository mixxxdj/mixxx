#pragma once

#include <QObject>

#include "control/controlobject.h"
#include "util/duration.h"
#include "util/memory.h"
#include "util/performancetimer.h"

// A helper class that manages the "guiTickTime" COs, that drive updates of the
// GUI from the VsyncThread at the user's configured FPS (possibly downsampled).
class GuiTick {
  public:
    GuiTick();
    void process();

  private:
    std::unique_ptr<ControlObject> m_pCOGuiTickTime;
    std::unique_ptr<ControlObject> m_pCOGuiTick50ms;
    PerformanceTimer m_cpuTimer;
    mixxx::Duration m_lastUpdateTime;
    mixxx::Duration m_cpuTimeLastTick;
};
