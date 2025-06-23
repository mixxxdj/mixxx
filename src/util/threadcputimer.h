#pragma once

#include "util/duration.h"

class ThreadCpuTimer {
  public:
    void start();
    mixxx::Duration elapsed() const;
    mixxx::Duration restart();
  private:
#if defined(Q_OS_UNIX)
    qint64 t1;
    qint64 t2;
#endif
};
