#include <mach/mach_init.h>
#include <mach/task_policy.h>
#include <mach/thread_act.h>
#include <mach/thread_policy.h>
#include <sys/sysctl.h>
int get_bus_speed(); // forward decl
void rtThread();

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

void rtThread()
{
    struct thread_time_constraint_policy ttcpolicy;
    kern_return_t theError;
    /* This is in AbsoluteTime units, which are equal to
       1/4 the bus speed on most machines. */
    // hard-coded numbers are approximations for 100 MHz bus speed.
    // assume that app deals in frame-sized chunks, e.g. 30 per second.
    // ttcpolicy.period=833333;
    ttcpolicy.period=(get_bus_speed() / 120);
    // ttcpolicy.computation=60000;
    ttcpolicy.computation=(get_bus_speed() / 1440);
    // ttcpolicy.constraint=120000;
    ttcpolicy.constraint=(get_bus_speed() / 720);
    ttcpolicy.preemptible=1;
    theError = thread_policy_set(mach_thread_self(),
               THREAD_TIME_CONSTRAINT_POLICY, (int *)&ttcpolicy,
               THREAD_TIME_CONSTRAINT_POLICY_COUNT);
#if SNDSTREAMCLIENT_DEBUG
    if (theError != KERN_SUCCESS)
        fprintf(stderr, "Can't do thread_policy_set\n");
#endif
}

