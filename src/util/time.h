#ifndef TIME_H
#define TIME_H

#include <QtGlobal>

#include "util/performancetimer.h"
#include "util/timer.h"

class Time {
  public:
    static void start() {
        s_timer.start();
    }

    // Returns the time elapsed since Mixxx started up in nanoseconds.
    static qint64 elapsed() {
        return s_timer.elapsed();
    }

  private:
    static PerformanceTimer s_timer;
};

#endif /* TIME_H */
