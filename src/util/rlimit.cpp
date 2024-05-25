#include "util/rlimit.h"

#ifdef __LINUX__

extern "C" {
    #include <sys/resource.h>
}

// TODO(xxx) this is the result from a calculation inside PortAudio
// We should query the value from PA or do the same calculations
constexpr rlim_t PA_RTPRIO = 82; // PA sets RtPrio = 82

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
    return (getCurRtPrio() >= PA_RTPRIO); // PA sets RtPrio = 82
}

#endif // __LINUX__
