/***************************************************************************
                          engineobject.cpp  -  description
                             -------------------
    begin                : Wed Feb 20 2002
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

#include "engineobject.h"

#ifdef __WIN__
#include "windows.h"
//using namespace System;
//using namespace System::Threading; 
#endif


// Static member variable definition
QString EngineObject::NAME_MASTER = 0;
QString EngineObject::NAME_HEAD = 0;
int EngineObject::SRATE = 0;
int EngineObject::BITS = 0;
int EngineObject::BUFFERSIZE = 0;
int EngineObject::CH_MASTER = 0;
int EngineObject::CH_HEAD = 0;
int EngineObject::NYQUIST = 0;
CSAMPLE EngineObject::norm = 0.;
FLOAT_TYPE EngineObject::BASERATE = 1.0;
MixxxView *EngineObject::view = 0;

EngineObject::EngineObject()
{
    //view = v;
}

EngineObject::~EngineObject()
{
}

void EngineObject::setParams(QString name, int srate, int bits, int bufferSize, int chMaster, int chHead)
{
    if (chMaster>0)
    {
        NAME_MASTER = name;
        CH_MASTER   = chMaster;
        CH_HEAD     = chHead;
    } else {
        NAME_HEAD   = name;
        CH_HEAD     = chHead;
    }
    SRATE      = srate;
    BITS       = bits;
    BUFFERSIZE = bufferSize;

    NYQUIST = SRATE/2;
    norm    = (2.*acos(-1.0))/SRATE;
    BASERATE = 44100.0/FLOAT_TYPE(SRATE); // Set the basic rate.
}

#ifdef __MACX__
int EngineObject::get_bus_speed()
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

void EngineObject::rtThread()
{
#ifdef __MACX__
    struct thread_time_constraint_policy ttcpolicy;
    kern_return_t theError;
    /* This is in AbsoluteTime units, which are equal to
       1/4 the bus speed on most machines. */
    // hard-coded numbers are approximations for 100 MHz bus speed.
    // assume that app deals in frame-sized chunks, e.g. 30 per second.
    //ttcpolicy.period=833333;
    ttcpolicy.period=(get_bus_speed() / 120);
    //ttcpolicy.computation=1000;
    ttcpolicy.computation=(get_bus_speed() / (30));
    //ttcpolicy.constraint=1500;
    ttcpolicy.constraint=(get_bus_speed() / (15));
    ttcpolicy.preemptible=1;
    theError = thread_policy_set(mach_thread_self(),
               THREAD_TIME_CONSTRAINT_POLICY, (int *)&ttcpolicy,
               THREAD_TIME_CONSTRAINT_POLICY_COUNT);
    if (theError != KERN_SUCCESS)
        fprintf(stderr, "Can't do thread_policy_set\n");
#endif
#ifdef __UNIX__
    // Try to set realtime priority on the current executing thread. This should be used in time-critical
    // producer threads.
    struct sched_param schp;
    memset(&schp, 0, sizeof(schp));

    // Choose a priority just one step lower than the PortAudio thread
    schp.sched_priority = ((sched_get_priority_max(SCHED_RR) - 11)); //sched_get_priority_min(SCHED_RR)) / 2)-1;
    if (sched_setscheduler(0, SCHED_RR, &schp) != 0)
    qWarning("Not possible to give audio producer thread high prioriy.");

#endif
#ifdef __WIN__
    HANDLE h = GetCurrentThread();
    SetThreadPriority(h,THREAD_PRIORITY_BELOW_NORMAL);
#endif
}
