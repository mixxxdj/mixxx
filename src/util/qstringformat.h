#pragma once

#include <QString>
#include <type_traits>
#include <utility>

namespace {

// taken from Qt
template<typename T>
static constexpr bool is_convertible_to_view_or_qstring_v =
        std::is_convertible_v<T, QString> ||
        std::is_convertible_v<T, QStringView> ||
        std::is_convertible_v<T, QLatin1String>;

// check if we can call QString::number(T) with T
template<typename T>
static constexpr bool is_number_compatible_v =
        std::is_invocable_v<decltype(QString::number(std::declval<T>()))(T), T>;

// always false, used for static_assert and workaround for compilers without
// https://cplusplus.github.io/CWG/issues/2518.html
template<typename T>
static constexpr bool always_false_v = false;
} // namespace

// Try to convert T to a type that would be accepted by QString::args(Args&&...)
template<typename T>
auto convertToQStringConvertible(T&& arg) {
    if constexpr (is_convertible_to_view_or_qstring_v<T>) {
        // no need to do anything, just return verbatim
        return std::forward<T>(arg);
    } else if constexpr (is_number_compatible_v<T>) {
        return QString::number(std::forward<T>(arg));
    } else {
        static_assert(always_false_v<T>, "Unsupported type for QString::arg");
        // unreachable, but returning a QString results in a better error message
        // because the log won't be spammed with all the QString::arg overloads
        // it couldn't match with `void`.
        return QString();
    }
}
