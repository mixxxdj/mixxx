
#include <QProcess>

#include "util/autohidpi.h"

#ifdef __WINDOWS__

double AutoHiDpi::getScaleFactor() {
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
