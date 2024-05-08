#pragma once

#include <span>
#include <type_traits>

#include "util/assert.h"

namespace mixxx {

/// Offers a group of utilities (functions) for working with std::span.
namespace spanutil {
/// The function casts data type of size to data type of size_type from std::span.
/// At the same time, the function provides appropriate lower bound checking
/// for signed data types.
template<typename T, typename S, typename T2 = typename std::span<T>::size_type>
constexpr T2 castToSizeType(S size) noexcept {
    if constexpr (std::is_signed_v<S> && std::is_unsigned_v<T2>) {
        VERIFY_OR_DEBUG_ASSERT(size >= 0) {
            size = 0;
        }
    }

    return static_cast<T2>(size);
}

/// The function creates std::span from pointer and size.
/// In most cases, the pointer to the raw data of a data structure
/// is used, and the size of the data structure.
template<typename T, typename S>
constexpr std::span<T> spanFromPtrLen(T* ptr, S size) noexcept {
    return std::span<T>{ptr, mixxx::spanutil::castToSizeType<T>(size)};
}

} // namespace spanutil

} // namespace mixxx
