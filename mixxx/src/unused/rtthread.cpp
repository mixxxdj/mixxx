/***************************************************************************
                          rtthread.cpp  -  description
                             -------------------
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                :
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "rtthread.h"

#ifdef __WINDOWS__
  #include "windows.h"
#endif
#include <string.h>
#include <qobject.h>
#include <QDebug>

#ifdef __APPLE__
int get_bus_speed()
{
    int mib[2];
    unsigned int miblen;
    int busspeed;
    int retval;
    size_t len;

    mib[0]=CTL_HW;
    mib[1]=HW_BUS_FREQ;
    miblen=2;
    len=4;
    retval = sysctl(mib, miblen, &busspeed, &len, NULL, 0);
    return busspeed;
}
#endif

void rtThread()
{
#ifdef __APPLE__
    struct thread_time_constraint_policy ttcpolicy;
    kern_return_t theError;
    /* This is in AbsoluteTime units, which are equal to
       1/4 the bus speed on most machines. */
    // hard-coded numbers are approximations for 100 MHz bus speed.
    // assume that app deals in frame-sized chunks, e.g. 30 per second.
    //ttcpolicy.period=833333;
    ttcpolicy.period=(get_bus_speed() / 120);
    //ttcpolicy.computation=1000;
    ttcpolicy.computation=(get_bus_speed() / (80));
    //ttcpolicy.constraint=1500;
    ttcpolicy.constraint=(get_bus_speed() / (1));
    ttcpolicy.preemptible=1;
    theError = thread_policy_set(mach_thread_self(),
                                 THREAD_TIME_CONSTRAINT_POLICY, (int *)&ttcpolicy,
                                 THREAD_TIME_CONSTRAINT_POLICY_COUNT);
    if (theError != KERN_SUCCESS)
        qDebug() << "Can't do thread_policy_set";
#endif
#ifdef __LINUX__
    // Try to set realtime priority on the current executing thread. This should be used in time-critical
    // producer threads.
    struct sched_param schp;
    memset(&schp, 0, sizeof(schp));

    // Choose a priority just one step lower than the PortAudio thread
    //schp.sched_priority = ((sched_get_priority_max(SCHED_RR) - 11)); //sched_get_priority_min(SCHED_RR)) / 2)-1;

    // Actually, for alsa real time priority is needed, and this function is only called in Linux by PlayerALSA
    schp.sched_priority = ((sched_get_priority_max(SCHED_RR))); //sched_get_priority_min(SCHED_RR)) / 2)-1;

    if (sched_setscheduler(0, SCHED_RR, &schp) != 0)
        qDebug() << "Not possible to give audio producer thread high prioriy.";
#endif
//#XXX FIXME: write versions of this function appropriate for BSD and OS X
#ifdef __WINDOWS__
//    HANDLE h = GetCurrentThread();
//    SetThreadPriority(h,THREAD_PRIORITY_BELOW_NORMAL);
#endif
}
