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

#ifdef Q_OS_MAC
IOPMAssertionID ScreenSaverHelper::systemSleepAssertionID=0;
IOPMAssertionID ScreenSaverHelper::userActivityAssertionID=0;

void ScreenSaverHelper::inhibit()
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
                                         &userActivityAssertionID);
        CFRelease(reasonForActivity);

        if (success != kIOReturnSuccess) {
            qWarning("failed to declare user activity.");
        }
    }

    /* prevent the system from sleeping */
    if (systemSleepAssertionID > 0) {
        qDebug() "IOKit releasing old screensaver inhibitor" << systemSleepAssertionID;
        IOPMAssertionRelease(systemSleepAssertionID);
    }

    IOReturn success;
    /* work-around a bug in 10.7.4 and 10.7.5, so check for 10.7.x < 10.7.4 and 10.8 */
    if (NSAppKitVersionNumber < 1115.2) {
        /* fall-back on the 10.5 mode, which also works on 10.7.4 and 10.7.5 */
        success = IOPMAssertionCreate(kIOPMAssertionTypeNoDisplaySleep, kIOPMAssertionLevelOn, 
                &systemSleepAssertionID);
    } else {
        CFStringRef reasonForActivity = CFStringCreateWithCString(kCFAllocatorDefault, 
                "Mixxx digital DJ software", kCFStringEncodingUTF8);
        success = IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep, kIOPMAssertionLevelOn, 
                reasonForActivity, &systemSleepAssertionID);
        CFRelease(reasonForActivity);
    }

    if (success == kIOReturnSuccess) {
        qDebug() << "IOKit screensaver inhibited " << systemSleepAssertionID;
    } else {
        qWarning("failed to prevent system sleep through IOKit");    
    }
}
void ScreenSaverHelper::uninhibit()
{
    /* allow the system to sleep again */
    if (systemSleepAssertionID > 0) {
        IOPMAssertionRelease(systemSleepAssertionID);
        qDebug() << "IOKit screensaver uninhibited " << systemSleepAssertionID;
    }
}

#elif defined(Q_OS_WIN)
void ScreenSaverHelper::inhibit()
{
    SetThreadExecutionState( ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED | ES_CONTINUOUS );
    qDebug() << "screensaver inhibited";
}
void ScreenSaverHelper::uninhibit()
{
    SetThreadExecutionState(ES_CONTINUOUS);
    qDebug() << "screensaver uninhibited";
}

#elif defined(Q_OS_LINUX)
const char *SCREENSAVERS[][4] = {
    // org.freedesktop.ScreenSaver is the standard. should work for kde.
    {"org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver", "Inhibit"},
    {"org.gnome.ScreenSaver", "/org/gnome/ScreenSaver", "org.gnome.ScreenSaver", "Inhibit"},
    // Seen this on internet, not sure if it was a typo or what.
    {"org.gnome.SessionManager", "/org/gnome/SessionManager", "org.gnome.SessionManager", "Inhibit"},
    // whatever...
    {"org.kde.screensaver", "/ScreenSaver", "org.kde.screensaver", "SimulateUserActivity"},
    {nullptr, nullptr, nullptr, nullptr}
};

uint32_t ScreenSaverHelper::cookie = 0;
int ScreenSaverHelper::saverindex = -1;

void ScreenSaverHelper::inhibit()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        qWarning("Cannot connect to the D-Bus session bus.\nTo start it, run:\n"
                "\teval `dbus-launch --auto-syntax`");
        return;
    }
    if (cookie > 0) {
        uninhibit();
    }
    cookie = 0;
    for (int i=0; SCREENSAVERS[i][0] != nullptr; i++ ) {
        QDBusInterface iface(SCREENSAVERS[i][0], SCREENSAVERS[i][1], SCREENSAVERS[i][2], 
            QDBusConnection::sessionBus());
        if (iface.isValid()) {
            QDBusReply<uint32_t> reply = iface.call("Inhibit", "org.mixxxdj","Mixxx active");
            if (reply.isValid()) {
                cookie = reply.value();
                saverindex = i;
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
void ScreenSaverHelper::uninhibit()
{
    if (cookie > 0) {
        QDBusInterface iface(SCREENSAVERS[saverindex][0], SCREENSAVERS[saverindex][1], 
            SCREENSAVERS[saverindex][2],  QDBusConnection::sessionBus());
        if (iface.isValid()) {
            QDBusReply<void> reply = iface.call("UnInhibit", cookie);
            if (reply.isValid()) {
                cookie = 0;
                qDebug() << "DBus screensaver " << SCREENSAVERS[saverindex][0] << " uninhibited";
            } else {
                qWarning() << "Call to uninhibit for " << SCREENSAVERS[saverindex][0] << " failed: " 
                    << reply.error().message();
            }
        } else {
            qDebug() << "DBus interface " << SCREENSAVERS[saverindex][0] << " not valid";
        }
    }
}

#elif HAS_XWINDOW_SCREENSAVER
void ScreenSaverHelper::inhibit()
{
    char *name = ":0.0";
    Display *display;
    if (getenv("DISPLAY"))
        name=getenv("DISPLAY");
    display=XOpenDisplay(name);
    XScreenSaverSuspend(display,True);
}
void ScreenSaverHelper::uninhibit()
{
    char *name = ":0.0";
    Display *display;
    if (getenv("DISPLAY"))
        name=getenv("DISPLAY");
    display=XOpenDisplay(name);
    XScreenSaverSuspend(, False);
}

#else
void ScreenSaverHelper::inhibit()
{
    qError("Screensaver suspending not implemented");
}
void ScreenSaverHelper::uninhibit()
{
    qError("Screensaver suspending not implemented");
}
#endif // Q_OS_MAC


} // namespace mixxx

