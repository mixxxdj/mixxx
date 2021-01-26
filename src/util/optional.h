#pragma once

#include <QtDebug>
#include <optional>

template<typename T>
QDebug operator<<(QDebug dbg, std::optional<T> arg) {
    if (arg) {
        return dbg << *arg;
    } else {
        return dbg << "nullopt";
    }
}
