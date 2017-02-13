#ifndef MIXXX_STRING_H
#define MIXXX_STRING_H


#include <QString>


// The default comparison of strings for sorting.
inline
int compareLocalAwareCaseInsensitive(
        const QString& first, const QString& second) {
    return QString::localeAwareCompare(first.toLower(), second.toLower());
}


#endif // MIXXX_STRING_H
