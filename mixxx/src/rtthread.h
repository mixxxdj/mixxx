/* Mixxx.
 *
 * Set real-time priority on a calling thread.
 *
 * Tue Haste Andersen <haste@diku.dk>, 2003.
 */

#ifndef RTTHREAD_H
#define RTTHREAD_H

#ifdef __MACX__
  #include <mach/mach_init.h>
  #include <mach/task_policy.h>
  #include <mach/thread_act.h>
  #include <mach/thread_policy.h>
  #include <sys/sysctl.h>
#endif
#ifdef __UNIX__
  #include <sched.h>
  #include <sys/time.h>
  #include <sys/resource.h>
#endif

#ifdef __MACX__
  int get_bus_speed();
#endif

void rtThread();

#endif
