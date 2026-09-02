#pragma once

#ifdef __LINUX__
#ifndef QT_NO_DBUS
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QString>
#endif
#endif

namespace mixxx {

// Detect if the desktop supports a global menu to decide whether we need to rebuild
// and reconnect the menu bar when switching to/from fullscreen mode.
// Compared to QMenuBar::isNativeMenuBar() (requires a set menu bar) and
// Qt::AA_DontUseNativeMenuBar, which may both change, this is way more reliable
// since it's rather unlikely that the Appmenu.Registrar service is unloaded/stopped
// while Mixxx is running.
// This is a reimplementation of QGenericUnixTheme > checkDBusGlobalMenuAvailable()
inline bool desktopSupportsGlobalMenuBar() {
#ifdef __LINUX__
#ifndef QT_NO_DBUS
    const QDBusConnection connection = QDBusConnection::sessionBus();
    if (const auto* pInterface = connection.interface()) {
        return pInterface->isServiceRegistered(
                QStringLiteral("com.canonical.AppMenu.Registrar"));
    }
#endif
#endif
    return false;
}

} // namespace mixxx
