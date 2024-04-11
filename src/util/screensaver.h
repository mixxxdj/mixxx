#pragma once

#include <QtGlobal>

#ifdef Q_OS_MACOS
#include <IOKit/pwr_mgt/IOPMLib.h>
#endif // Q_OS_MACOS

namespace mixxx {

// Code related to interacting with the screensaver.
//
// Main use is to prevent the screensaver from starting if Mixxx is being used.
//
class ScreenSaverHelper {
public:

   static void inhibit();
   static void uninhibit();
   static void triggerUserActivity();

private:
   static void inhibitInternal();
   static void uninhibitInternal();

   static bool s_enabled;
   static bool s_sendActivity;
#if defined(Q_OS_MACOS)
   /* sleep management */
   static IOPMAssertionID s_systemSleepAssertionID;
   static IOPMAssertionID s_userActivityAssertionID;
#elif defined(Q_OS_LINUX)
   static QString s_session_handle;
   static uint32_t s_inhibit_cookie;
#endif // Q_OS_MACOS
};

}
