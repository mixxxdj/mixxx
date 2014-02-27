#ifndef THREADCPUTIMER_H
#define THREADCPUTIMER_H

#include "util/performancetimer.h"

class ThreadCpuTimer
{
public:
    void start();
    qint64 elapsed() const;
    qint64 restart();
private:
    qint64 t1;
    qint64 t2;
};

#endif // PERFORMANCETIMER_H
