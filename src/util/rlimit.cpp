#include "rlimit.h"

#ifdef __LINUX__

extern "C" {
    #include <sys/time.h>
    #include <sys/resource.h>
}

// static
unsigned int RLimit::getCurRtPrio() {
    struct rlimit limits;
    if(getrlimit(RLIMIT_RTPRIO, &limits)) {
        // Error
        return 100;
    }
    return limits.rlim_cur;
}

// static
unsigned int RLimit::getMaxRtPrio() {
    struct rlimit limits;
    if(getrlimit(RLIMIT_RTPRIO, &limits)) {
        // Error
        return 0;
    }
    return limits.rlim_max;
}

// static
bool RLimit::isRtPrioAllowed() {
    return (getCurRtPrio() >= 82); // PA sets RtPrio = 82
}

#endif // __LINUX__
