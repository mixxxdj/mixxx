#pragma once

#include <QtDebug>
// support for QDebugging std::optional<T> has been added natively in Qt 6.7
// this definition clashes with Qt's so disable it.
#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)

#include <optional>

template<typename T>
inline QDebug operator<<(QDebug dbg, std::optional<T> arg) {
    if (arg) {
        return dbg << *arg;
    } else {
        return dbg << "nullopt";
    }
}

#endif
