#pragma once

#if __has_include(<optional>) || \
    (defined(__cpp_lib_optional) && __cpp_lib_optional >= 201606L)

#include <optional>

#else

#ifdef __clang__
#pragma clang diagnostic ignored "-Winvalid-constexpr"
#endif

#include <experimental/optional>

namespace std {

using std::experimental::make_optional;
using std::experimental::nullopt;
using std::experimental::optional;

// Workarounds for missing member functions:
// option::has_value() -> explicit operator bool()
// option::value() -> operator*()

} // namespace std

#endif

#include <QtDebug>

template<typename T>
QDebug operator<<(QDebug dbg, std::optional<T> arg) {
    if (arg) {
        return dbg << *arg;
    } else {
        return dbg << "nullopt";
    }
}
