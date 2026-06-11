#pragma once

#include "util/performancetimer.h"

class VSyncTimeProvider {
  public:
    virtual std::chrono::microseconds fromTimerToNextSync(const PerformanceTimer& timer) = 0;
    virtual std::chrono::microseconds getSyncInterval() const = 0;
};
