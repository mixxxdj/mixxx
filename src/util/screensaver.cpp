#include <QtDebug>

/**
Documentation:
OSX: https://developer.apple.com/reference/iokit/1557134-iopmassertioncreatewithname
Windows: https://msdn.microsoft.com/en-us/library/windows/desktop/aa373208(v=vs.85).aspx
Freedesktop: https://people.freedesktop.org/~hadess/idle-inhibition-spec/re01.html
XScreenSaver: https://linux.die.net/man/3/xscreensaversuspend
GTK: https://developer.gnome.org/gtk3/stable/GtkApplication.html#gtk-application-inhibit

With the help of the following source codes:

https://github.com/videolan/vlc/blob/fbaa27ae2d7fcf5ccee7f0ca424ec0cc5bf01f4c/modules/gui/macosx/VLCInputManager.m#L299
https://github.com/videolan/vlc/blob/fbaa27ae2d7fcf5ccee7f0ca424ec0cc5bf01f4c/modules/video_output/win32/events.c#L168
https://github.com/GNOME/totem/blob/0c18deceed780e5ca13ba2b97446d81d70cfbec6/src/plugins/screensaver/totem-screensaver.c#L76
https://github.com/awjackson/bsnes-classic/blob/038e2e051ffc8abe7c56a3bf27e3016c433ee563/bsnes/ui-qt/platform/platform_x.cpp

**/

#include "util/screensaver.h"

#if defined(Q_OS_MAC)
#  include "util/mac.h"
#elif defined(Q_OS_WIN)
#  include <windows.h>
#elif defined(Q_OS_LINUX)
#  include <QtDBus>
#elif HAVE_XSCREENSAVER_SUSPEND
#  include <X11/extensions/scrnsaver.h>
#endif // Q_OS_WIN


namespace mixxx {

bool ScreenSaverHelper::s_enabled = false;

// inhibitInternal and unihibitInternal should be called the same amount of times
// depending on the implementation used. This ensures it.
void ScreenSaverHelper::inhibit()
{
    if (!s_enabled) {
        inhibit();
    }
}
void ScreenSaverHelper::uninhibit()
{
    if (s_enabled) {
        uninhibit();
    }
}


#ifdef Q_OS_MAC
IOPMAssertionID ScreenSaverHelper::s_systemSleepAssertionID=0;
IOPMAssertionID ScreenSaverHelper::s_userActivityAssertionID=0;

void ScreenSaverHelper::inhibitInternal()
{
    /* Declare user activity.
     This wakes the display if it is off, and postpones display sleep according to the users system preferences
     Available from 10.7.3 */
    if (&IOPMAssertionDeclareUserActivity)
    {
        CFStringRef reasonForActivity = CFStringCreateWithCString(kCFAllocatorDefault, 
                "Mixxx digital DJ software", kCFStringEncodingUTF8);
        IOReturn success = IOPMAssertionDeclareUserActivity(reasonForActivity,
                                         kIOPMUserActiveLocal,
                                         &s_userActivityAssertionID);
        CFRelease(reasonForActivity);

        if (success != kIOReturnSuccess) {
            qWarning("failed to declare user activity.");
        }
    }

    /* prevent the system from sleeping */
    if (s_systemSleepAssertionID > 0) {
        qDebug() << "IOKit releasing old screensaver inhibitor" << s_systemSleepAssertionID;
        IOPMAssertionRelease(s_systemSleepAssertionID);
    }

    IOReturn success;
    CFStringRef reasonForActivity = CFStringCreateWithCString(kCFAllocatorDefault, 
            "Mixxx digital DJ software", kCFStringEncodingUTF8);
    success = IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep, kIOPMAssertionLevelOn, 
            reasonForActivity, &s_systemSleepAssertionID);
    CFRelease(reasonForActivity);

    if (success == kIOReturnSuccess) {
        s_enabled = true;
        qDebug() << "IOKit screensaver inhibited " << s_systemSleepAssertionID;
    } else {
        qWarning("failed to prevent system sleep through IOKit");    
    }
}
void ScreenSaverHelper::uninhibitInternal()
{
    /* allow the system to sleep again */
    if (s_systemSleepAssertionID > 0) {
        s_enabled = false;
        qDebug() << "IOKit screensaver uninhibited " << s_systemSleepAssertionID;
        IOPMAssertionRelease(s_systemSleepAssertionID);
    }
}

#elif defined(Q_OS_WIN)
void ScreenSaverHelper::inhibitInternal()
{
    // Calling once without "ES_CONTINUOUS" to force the monitor to wake up.
    SetThreadExecutionState( ES_DISPLAY_REQUIRED );
    SetThreadExecutionState( ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED | ES_CONTINUOUS );
    qDebug() << "screensaver inhibited";
    s_enabled = true;
}
void ScreenSaverHelper::uninhibitInternal()
{
    SetThreadExecutionState(ES_CONTINUOUS);
    qDebug() << "screensaver uninhibited";
    s_enabled = false;
}

#elif defined(Q_OS_LINUX)
const char *SCREENSAVERS[][4] = {
    // org.freedesktop.ScreenSaver is the standard. should work for kde.
    {"org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver", "Inhibit"},
    {"org.gnome.ScreenSaver", "/org/gnome/ScreenSaver", "org.gnome.ScreenSaver", "Inhibit"},
    // Seen this on internet, not sure if it was a typo or what.
    {"org.gnome.SessionManager", "/org/gnome/SessionManager", "org.gnome.SessionManager", "Inhibit"},
    // This can be used to simulate action instead. gnome also has "SimulateUserActivity".
    {"org.kde.screensaver", "/ScreenSaver", "org.kde.screensaver", "SimulateUserActivity"},
    {nullptr, nullptr, nullptr, nullptr}
};

uint32_t ScreenSaverHelper::s_cookie = 0;
int ScreenSaverHelper::s_saverindex = -1;

void ScreenSaverHelper::inhibitInternal()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        qWarning("Cannot connect to the D-Bus session bus.\nTo start it, run:\n"
                "\teval `dbus-launch --auto-syntax`");
        return;
    }
    if (s_cookie > 0) {
        uninhibit();
    }
    s_cookie = 0;
    for (int i=0; SCREENSAVERS[i][0] != nullptr; i++ ) {
        QDBusInterface iface(SCREENSAVERS[i][0], SCREENSAVERS[i][1], SCREENSAVERS[i][2], 
            QDBusConnection::sessionBus());
        if (iface.isValid()) {
            QDBusReply<uint32_t> reply = iface.call("Inhibit", "org.mixxxdj","Mixxx active");
            if (reply.isValid()) {
                s_cookie = reply.value();
                s_saverindex = i;
                s_enabled = true;
                qDebug() << "DBus screensaver " << SCREENSAVERS[i][0] <<" inhibited";
                break;
            } else {
                qWarning() << "Call to inhibit for " << SCREENSAVERS[i][0] << " failed: " 
                    << reply.error().message();
                return;
            }
        } else {
            qDebug() << "DBus interface " << SCREENSAVERS[i][0] << " not valid";
        }
    }
}
void ScreenSaverHelper::uninhibitInternal()
{
    if (s_cookie > 0) {
        s_enabled = false;
        QDBusInterface iface(SCREENSAVERS[s_saverindex][0], SCREENSAVERS[s_saverindex][1], 
            SCREENSAVERS[s_saverindex][2],  QDBusConnection::sessionBus());
        if (iface.isValid()) {
            QDBusReply<void> reply = iface.call("UnInhibit", s_cookie);
            if (reply.isValid()) {
                s_cookie = 0;
                qDebug() << "DBus screensaver " << SCREENSAVERS[s_saverindex][0] << " uninhibited";
            } else {
                qWarning() << "Call to uninhibit for " << SCREENSAVERS[s_saverindex][0] << " failed: " 
                    << reply.error().message();
            }
        } else {
            qDebug() << "DBus interface " << SCREENSAVERS[s_saverindex][0] << " not valid";
        }
    }
}

#elif HAS_XWINDOW_SCREENSAVER
// This is untested.
void ScreenSaverHelper::inhibitInternal()
{
    char *name = ":0.0";
    Display *display;
    if (getenv("DISPLAY"))
        name=getenv("DISPLAY");
    display=XOpenDisplay(name);
    XScreenSaverSuspend(display,True);
    s_enabled = true;
}
void ScreenSaverHelper::uninhibitInternal()
{
    char *name = ":0.0";
    Display *display;
    if (getenv("DISPLAY"))
        name=getenv("DISPLAY");
    display=XOpenDisplay(name);
    XScreenSaverSuspend(display, False);
    s_enabled = false;
}

#else
void ScreenSaverHelper::inhibitInternal()
{
    qError("Screensaver suspending not implemented");
}
void ScreenSaverHelper::uninhibitInternal()
{
    qError("Screensaver suspending not implemented");
}
#endif // Q_OS_MAC


} // namespace mixxx

