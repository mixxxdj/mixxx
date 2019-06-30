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
#include "util/assert.h"

#if defined(Q_OS_MAC)
#  include "util/mac.h"
#elif defined(Q_OS_WIN)
#  include <windows.h>
#elif defined(Q_OS_LINUX)
#  include <QtDBus>
#elif HAVE_XSCREENSAVER_SUSPEND
#  include <X11/extensions/scrnsaver.h>
#endif // Q_OS_WIN

#if defined(Q_OS_LINUX) || HAVE_XSCREENSAVER_SUSPEND
#  define None XNone
#  define Window XWindow
#  include <X11/Xlib.h>
#  undef None
#  undef Window
#endif

namespace mixxx {

bool ScreenSaverHelper::s_enabled = false;

// inhibitInternal and unihibitInternal should be called the same amount of times
// depending on the implementation used. This ensures it.
void ScreenSaverHelper::inhibit()
{
    if (!s_enabled) {
        inhibitInternal();
    }
}
void ScreenSaverHelper::uninhibit()
{
    if (s_enabled) {
        uninhibitInternal();
    }
}


#ifdef Q_OS_MAC
IOPMAssertionID ScreenSaverHelper::s_systemSleepAssertionID=0;
IOPMAssertionID ScreenSaverHelper::s_userActivityAssertionID=0;

void ScreenSaverHelper::triggerUserActivity()
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
}

void ScreenSaverHelper::inhibitInternal()
{
     triggerUserActivity();
 
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
    // JosepMaJAZ: I am not sure if this needs to be called or not, and specifically when
    // called on on the triggerUserActivity alone.
    if (s_userActivityAssertionID > 0) {
        IOPMAssertionRelease(s_userActivityAssertionID);
    }
}

#elif defined(Q_OS_WIN)
void ScreenSaverHelper::triggerUserActivity()
{
    SetThreadExecutionState( ES_DISPLAY_REQUIRED );
}
void ScreenSaverHelper::inhibitInternal()
{
    // Calling once without "ES_CONTINUOUS" to force the monitor to wake up.
    triggerUserActivity();
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
    // org.freedesktop.ScreenSaver is the standard. should work for gnome and kde too, 
    // but I add their specific names too
    {"org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver", "Inhibit"},
    {"org.gnome.ScreenSaver", "/org/gnome/ScreenSaver", "org.gnome.ScreenSaver", "Inhibit"},
    {"org.kde.screensaver", "/ScreenSaver", "org.kde.screensaver", "Inhibit"},
    {nullptr, nullptr, nullptr, nullptr}
};
// Disabling the method with DBus since it seems to be failing on several systems.
#if 0
const char *USERACTIVITY[][4] = {
    // "SimulateUserActivity" is not widely supported, but we can try that first.
    {"org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver", "SimulateUserActivity" },
    {"org.cinnamon.ScreenSaver", "/ScreenSaver", "org.cinnamon.ScreenSaver", "SimulateUserActivity"},
    {"org.gnome.ScreenSaver", "/org/gnome/ScreenSaver", "org.gnome.ScreenSaver", "SimulateUserActivity"},
    {"org.kde.screensaver", "/ScreenSaver", "org.kde.screensaver", "SimulateUserActivity"},
    {nullptr, nullptr, nullptr, nullptr}
};
#endif // 0

uint32_t ScreenSaverHelper::s_cookie = 0;
int ScreenSaverHelper::s_saverindex = -1;
bool ScreenSaverHelper::s_sendActivity = true;

void ScreenSaverHelper::triggerUserActivity()
{
    const char* name = ":0.0";
    Display *display;
    if (getenv("DISPLAY"))
        name=getenv("DISPLAY");
    display=XOpenDisplay(name);
    XResetScreenSaver(display);
    XCloseDisplay(display);
    return;
}
// Disabling the method with DBus since it seems to be failing on several systems.
#if 0
void ScreenSaverHelper::triggerUserActivity()
{
    if (!s_sendActivity) {
        // If the D-Bus method didn't work, let's try the Xlib method.
        const char* name = ":0.0";
        Display *display;
        if (getenv("DISPLAY"))
            name=getenv("DISPLAY");
        display=XOpenDisplay(name);
        XResetScreenSaver(display);
        XCloseDisplay(display);
        return;
    }
    
    s_sendActivity = false;
    if (!QDBusConnection::sessionBus().isConnected()) {
        qWarning("Cannot connect to the D-Bus session bus.\nTo start it, run:\n"
                "\teval `dbus-launch --auto-syntax`");
        return;
    }
    s_sendActivity = false;
    QString errors;
    for (int i=0; USERACTIVITY[i][0] != nullptr; i++ ) {
        QDBusInterface iface(USERACTIVITY[i][0], USERACTIVITY[i][1], USERACTIVITY[i][2], 
            QDBusConnection::sessionBus());
        if (iface.isValid()) {
            QDBusReply<void> reply = iface.call(USERACTIVITY[i][3]);
            if (reply.isValid()) {
                s_sendActivity = true;
                break;
            } else {
                errors = errors +  "\nCall to inhibit for " + USERACTIVITY[i][0] + " failed: " 
                    + reply.error().message();
            }
        } 
    }
    if (!s_sendActivity) {
        qWarning() << "Could not send activity using the registered DBus methods. " 
        << "Errors were: " << errors << 
        "\nWill try to use the Xlib XResetScreensaver instead.";
    }
}
#endif // 0

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
            QDBusReply<uint32_t> reply = iface.call(SCREENSAVERS[i][3], "org.mixxxdj","Mixxx active");
            if (reply.isValid()) {
                s_cookie = reply.value();
                s_saverindex = i;
                s_enabled = true;
                qDebug() << "DBus screensaver " << SCREENSAVERS[i][0] <<" inhibited";
                break;
            } else {
                qWarning() << "Call to inhibit for " << SCREENSAVERS[i][0] << " failed: " 
                    << reply.error().message();
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
struct LibXtst : public library {
  function<int (Display*, unsigned int, Bool, unsigned long)> XTestFakeKeyEvent;

  LibXtst() {
    if(open("Xtst")) {
      XTestFakeKeyEvent = sym("XTestFakeKeyEvent");
    }
  }
} libXtst;

void ScreenSaverHelper::triggerUserActivity()
{
    char *name = ":0.0";
    Display *display;
    if (getenv("DISPLAY"))
        name=getenv("DISPLAY");
    display=XOpenDisplay(name);
    libXtst.XTestFakeKeyEvent(display, 255, True,  0);
    libXtst.XTestFakeKeyEvent(display, 255, False, 0);
    XCloseDisplay(display);
}
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
void ScreenSaverHelper::triggerUserActivity()
{
    DEBUG_ASSERT(!"Screensaver trigger activity not implemented");
}
void ScreenSaverHelper::inhibitInternal()
{
    DEBUG_ASSERT(!"Screensaver suspending not implemented");
}
void ScreenSaverHelper::uninhibitInternal()
{
    DEBUG_ASSERT(!"Screensaver suspending not implemented");
}
#endif // Q_OS_MAC


} // namespace mixxx

