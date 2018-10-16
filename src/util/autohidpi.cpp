
#include <QProcess>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>

#include "util/autohidpi.h"

#ifdef __WINDOWS__

double AutoHiDpi::getScaleFactor() {
    // Windows 10 has a percentage setting in preferences, which
    // finally changes the screen DPI setting
    // 96 dpi = 100%
    // 120 dpi = 125%
    // 144 dpi = 150%
    // 192 dpi = 200%
    // 240 dpi = 250%
    // 288 dpi = 300%
    int dpiX = QApplication::desktop()->logicalDpiX();
    qDebug() << "AutoHiDpi::getScaleFactor()" << dpiX;
    // This follows the strategy used in Gnome/Unity
    // that the widget size is doubled once the font size
    // reaches the double vale threshold
    if (dpiX < 96) {
        return -1;
    } else if (dpiX < 192) {
        return 1;
    } else if (dpiX < 288) {
        return 2;
    } else if (dpiX == 288) {
        return 3;
    }
    return -1;
}

#elif __APPLE__

double AutoHiDpi::getScaleFactor() {
    return -1;
}

#else

double AutoHiDpi::getScaleFactor() {
    QProcess gsettingsProcess;
    QStringList args;
    args.append("get");
    args.append("org.gnome.desktop.interface");
    args.append("scaling-factor");
    gsettingsProcess.start("gsettings", args);
    gsettingsProcess.waitForFinished();
    QString output = gsettingsProcess.readAllStandardOutput();
    // output is now "uint32 1" or "" if gsettings is not installed
    bool ok;
    double value = output.remove(0, 7).toDouble(&ok);
    if (ok && value > 0) {
        return value;
    }
    return 0;
}

#endif // __LINUX__
