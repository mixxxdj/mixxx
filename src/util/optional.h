#pragma once

#include <optional>

#include <QtDebug>

template<typename T>
QDebug operator<<(QDebug dbg, std::optional<T> arg) {
    if (arg) {
        return dbg << *arg;
    } else {
        return dbg << "nullopt";
    }
}
