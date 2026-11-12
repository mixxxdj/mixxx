/**
Documentation:
OSX:
https://developer.apple.com/reference/iokit/1557134-iopmassertioncreatewithname
Windows:
https://msdn.microsoft.com/en-us/library/windows/desktop/aa373208(v=vs.85).aspx
Freedesktop:
https://people.freedesktop.org/~hadess/idle-inhibition-spec/re01.html
XScreenSaver: https://linux.die.net/man/3/xscreensaversuspend
GTK:
https://developer.gnome.org/gtk3/stable/GtkApplication.html#gtk-application-inhibit
Portal:
https://docs.flatpak.org/en/latest/portal-api-reference.html#gdbus-org.freedesktop.portal.Inhibit

With the help of the following source codes:

https://github.com/videolan/vlc/blob/fbaa27ae2d7fcf5ccee7f0ca424ec0cc5bf01f4c/modules/gui/macosx/VLCInputManager.m#L299
https://github.com/videolan/vlc/blob/fbaa27ae2d7fcf5ccee7f0ca424ec0cc5bf01f4c/modules/video_output/win32/events.c#L168
https://github.com/GNOME/totem/blob/0c18deceed780e5ca13ba2b97446d81d70cfbec6/src/plugins/screensaver/totem-screensaver.c#L76
https://github.com/awjackson/bsnes-classic/blob/038e2e051ffc8abe7c56a3bf27e3016c433ee563/bsnes/ui-qt/platform/platform_x.cpp
https://github.com/libsdl-org/SDL/blob/70b0d33106e98176bf44de5d301855d49587fa50/src/core/linux/SDL_dbus.c#L428

**/

#include "util/screensaver.h"

#include <QDebug>
#include <QtGlobal>

#include "util/assert.h"

#if defined(Q_OS_MACOS)
#  include "util/mac.h"
#elif defined(Q_OS_IOS)
#include "util/screensaverios.h"
#elif defined(_WIN32)
#  include <windows.h>
#elif defined(__LINUX__)
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#elif defined(HAVE_XSCREENSAVER_SUSPEND) && HAVE_XSCREENSAVER_SUSPEND
#  include <X11/extensions/scrnsaver.h>
#endif

#if defined(__LINUX__) || (defined(HAVE_XSCREENSAVER_SUSPEND) && HAVE_XSCREENSAVER_SUSPEND)
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

#ifdef Q_OS_MACOS
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
// Common inhibitor structure
struct Inhibitor {
    QString service;
    QString object_path;
    QString interface;
};

// Portal idle ihibitor
const Inhibitor kFreeDesktopPortal = {
        .service = QStringLiteral("org.freedesktop.portal.Desktop"),
        .object_path = QStringLiteral("/org/freedesktop/portal/desktop"),
        .interface = QStringLiteral("org.freedesktop.portal.Inhibit")};

// Standard D-Bus idle inhibitors
const std::array<Inhibitor, 6> kDBusInhibitors = {
        {{.service = QStringLiteral("org.freedesktop.ScreenSaver"),
                 .object_path = QStringLiteral("/org/freedesktop/ScreenSaver"),
                 .interface = QStringLiteral("org.freedesktop.ScreenSaver")},
                {.service = QStringLiteral("org.gnome.ScreenSaver"),
                        .object_path = QStringLiteral("/org/gnome/ScreenSaver"),
                        .interface = QStringLiteral("org.gnome.ScreenSaver")},
                {.service = QStringLiteral("org.kde.ScreenSaver"),
                        .object_path = QStringLiteral("/org/kde/ScreenSaver"),
                        .interface = QStringLiteral("org.kde.ScreenSaver")},
                {.service = QStringLiteral("org.cinnamon.ScreenSaver"),
                        .object_path = QStringLiteral("/org/cinnamon/ScreenSaver"),
                        .interface = QStringLiteral("org.cinnamon.ScreenSaver")},
                {.service = QStringLiteral("org.mate.ScreenSaver"),
                        .object_path = QStringLiteral("/org/mate/ScreenSaver"),
                        .interface = QStringLiteral("org.mate.ScreenSaver")},
                {.service = QStringLiteral("org.xfce.ScreenSaver"),
                        .object_path = QStringLiteral("/org/xfce/ScreenSaver"),
                        .interface = QStringLiteral("org.xfce.ScreenSaver")}}};

// Non-const pointer to selected constant inhibitor
static const Inhibitor* s_inhibitor = nullptr;

// Inhibit portal handle
QString ScreenSaverHelper::s_session_handle = "";

// Standard inhibit cookie
uint32_t ScreenSaverHelper::s_inhibit_cookie = 0;

void ScreenSaverHelper::triggerUserActivity()
{
    const char* name = ":0.0";
    Display *display;
    if (getenv("DISPLAY")) {
        name=getenv("DISPLAY");
    }
    display=XOpenDisplay(name);
    if (display != nullptr) {
        XResetScreenSaver(display);
        XCloseDisplay(display);
    }
    return;
}

void ScreenSaverHelper::inhibitInternal() {
    if (!QDBusConnection::sessionBus().isConnected()) {
        qWarning() << "Cannot connect to the D-Bus session bus";
        return;
    }

    // Call uninhibit if the handle / cookie has not been reset
    if (!s_session_handle.isEmpty() || s_inhibit_cookie > 0) {
        uninhibit();
    }

    for (const Inhibitor& inhibitor : kDBusInhibitors) {
        // Iterate through D-Bus idle inhibitors and select the first available
        QDBusInterface iface(inhibitor.service,
                inhibitor.object_path,
                inhibitor.interface,
                QDBusConnection::sessionBus());
        if (iface.isValid()) {
            QDBusReply<uint32_t> reply = iface.call("Inhibit",
                    "org.mixxx.Mixxx",
                    "Mixxx is running");
            if (reply.isValid()) {
                s_inhibitor = &inhibitor;
                s_inhibit_cookie = reply.value();
                s_enabled = true;
                qDebug() << "Idle inhibitor" << inhibitor.interface << "enabled";
                return;
            } else {
                qWarning() << "Inhibit for"
                           << inhibitor.interface << "failed: "
                           << reply.error().message();
            }
        } else {
            qDebug() << "Interface" << inhibitor.interface << "is not valid";
        }
    }

    // No standard D-Bus inhibitors found, so check if we're running inside a
    // Flatpak sandbox and try to use the portal instead
    if (std::getenv("container") && std::string(std::getenv("container")) == "flatpak") {
        QDBusInterface iface(kFreeDesktopPortal.service,
                kFreeDesktopPortal.object_path,
                kFreeDesktopPortal.interface,
                QDBusConnection::sessionBus());
        if (iface.isValid()) {
            // Portal API: 8 = idle inhibit
            QDBusReply<QDBusObjectPath> reply = iface.call("Inhibit",
                    "",
                    static_cast<uint32_t>(8),
                    QVariantMap{{"reason", "Mixxx is running"}});
            if (reply.isValid()) {
                s_inhibitor = &kFreeDesktopPortal;
                s_session_handle = reply.value().path();
                s_enabled = true;
                qDebug() << "Idle inhibitor" << kFreeDesktopPortal.interface << "enabled";
                return;
            } else {
                qWarning() << "Inhibit for" << kFreeDesktopPortal.interface << "failed: "
                           << reply.error().message();
            }
        } else {
            qDebug() << "Interface" << kFreeDesktopPortal.interface << "is not valid";
        }
    }

    if (!s_inhibitor) {
        qDebug() << "No supported idle inhibitation services found";
    }
}

void ScreenSaverHelper::uninhibitInternal() {
    if (!s_inhibitor) {
        qDebug() << "Cannot uninhibit without a selected interface";
        return;
    }

    if (s_inhibitor->service == "org.freedesktop.portal.Desktop" && !s_session_handle.isEmpty()) {
        // Ihibit portal uses the common request interface for uninhibit
        QDBusInterface iface(s_inhibitor->service,
                s_session_handle,
                "org.freedesktop.portal.Request",
                QDBusConnection::sessionBus());
        if (iface.isValid()) {
            QDBusReply<void> reply = iface.call("Close");
            if (reply.isValid()) {
                s_session_handle = "";
                s_enabled = false;
                qDebug() << "Idle inhibitor" << s_inhibitor->interface << "disabled";
            } else {
                qWarning() << "Call to uninhibit for"
                           << s_inhibitor->interface << "failed:"
                           << reply.error().message();
            }
        } else {
            qDebug() << "Interface" << s_inhibitor->interface << "is not valid";
        }
    } else if (s_inhibit_cookie > 0) {
        // Standard D-Bus idle uninhibit
        QDBusInterface iface(s_inhibitor->service,
                s_inhibitor->object_path,
                s_inhibitor->interface,
                QDBusConnection::sessionBus());
        if (iface.isValid()) {
            QDBusReply<void> reply = iface.call("UnInhibit", s_inhibit_cookie);
            if (reply.isValid()) {
                s_inhibit_cookie = 0;
                s_enabled = false;
                qDebug() << "Idle inhibitor" << s_inhibitor->interface << "disabled";
            } else {
                qWarning() << "Call to uninhibit for"
                           << s_inhibitor->interface << "failed:"
                           << reply.error().message();
            }
        } else {
            qDebug() << "Interface" << s_inhibitor->interface << "is not valid";
        }
    }
}

#elif defined(HAS_XWINDOW_SCREENSAVER) && HAS_XWINDOW_SCREENSAVER
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

#elif defined(Q_OS_IOS)
void ScreenSaverHelper::triggerUserActivity() {
}
void ScreenSaverHelper::inhibitInternal() {
    setIdleTimerDisabled(true);
    s_enabled = true;
}
void ScreenSaverHelper::uninhibitInternal() {
    setIdleTimerDisabled(false);
    s_enabled = false;
}
#elif defined(Q_OS_WASM)
// Screensavers are not supported
void ScreenSaverHelper::triggerUserActivity() {
}
void ScreenSaverHelper::inhibitInternal() {
}
void ScreenSaverHelper::uninhibitInternal() {
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
#endif // Q_OS_MACOS

} // namespace mixxx
