#pragma once

#include "util/performancetimer.h"

class ISyncTimeProvider {
  public:
    virtual int fromTimerToNextSyncMicros(const PerformanceTimer& timer) = 0;
    virtual int getSyncIntervalTimeMicros() const = 0;
};
