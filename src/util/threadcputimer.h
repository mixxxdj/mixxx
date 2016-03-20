#ifndef UTIL_THREADCPUTIMER_H
#define UTIL_THREADCPUTIMER_H

#include "util/duration.h"
#include "util/performancetimer.h"

class ThreadCpuTimer {
  public:
    void start();
    mixxx::Duration elapsed() const;
    mixxx::Duration restart();
  private:
    qint64 t1;
    qint64 t2;
};

#endif // UTIL_THREADCPUTIMER_H
