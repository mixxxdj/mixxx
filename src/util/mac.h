// Platform-specific helpers for OS X and iOS.
#ifndef MAC_H
#define MAC_H

#include <QtGlobal>

#ifdef Q_OS_MAC
#include <CoreFoundation/CFString.h>

#include <QString>

QString CFStringToQString(CFStringRef str);
CFStringRef QStringToCFString(const QString& str);
#endif

#endif /* MAC_H */
