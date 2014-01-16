#include "util/mac.h"

#ifdef Q_OS_MAC
QString CFStringToQString(CFStringRef str) {
    if (!str)
        return QString();

    CFIndex length = CFStringGetLength(str);
    if (length == 0)
        return QString();

    QString string(length, Qt::Uninitialized);
    CFStringGetCharacters(str, CFRangeMake(0, length), reinterpret_cast<UniChar *>
        (const_cast<QChar *>(string.unicode())));
    return string;
}
#endif
