// Platform-specific helpers for OS X and iOS.
#pragma once

#include <QtGlobal>

#ifdef Q_OS_MAC
#include <QString>
#include <CoreFoundation/CFString.h>

QString CFStringToQString(CFStringRef str);
CFStringRef QStringToCFString(const QString& str);
#endif
