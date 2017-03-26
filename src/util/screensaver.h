#ifndef MIXXX_SCREENSAVER_H
#define MIXXX_SCREENSAVER_H

#ifdef Q_OS_MAC
#import <IOKit/pwr_mgt/IOPMLib.h>
#endif // Q_OS_MAC

namespace mixxx {

// Code related to interacting with the screensaver. 
//
// Main use is to prevent the screensaver from starting if Mixxx is being used.
//
class ScreenSaverHelper {
public:

   static void inhibit();
   static void uninhibit();
   static void inhibitOnCondition(bool desired);

private:
   static bool enabled;
#if defined(Q_OS_MAC)
    /* sleep management */
    static IOPMAssertionID systemSleepAssertionID;
    static IOPMAssertionID userActivityAssertionID;
#elif defined(Q_OS_LINUX)
    static uint32_t cookie;
    static int saverindex;
#endif // Q_OS_MAC
};

}

#endif // MIXXX_SCREENSAVER_H
