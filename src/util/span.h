#pragma once

#include <span>
#include <type_traits>

#include "util/assert.h"

namespace mixxx {

/// The class offers a group of utilities (methods) for working with std::span.
class SpanUtil {
  public:
    /// The method creates std::span from pointer and size.
    /// In most cases, the pointer to the raw data of a data structure
    /// is used, and the size of the data structure.
    template<typename T, typename S>
    static std::span<T> spanFromPtrLen(T* ptr, S size) {
        return std::span<T>{ptr, castToSizeType<T>(size)};
    }

  private:
    /// The method casts data type of size to data type of size_type from std::span.
    /// At the same time, the method provides appropriate lower bound checking.
    template<typename T, typename S, typename T2 = typename std::span<T>::size_type>
    static T2 castToSizeType(S size) {
        if constexpr (std::is_signed_v<S>) {
            VERIFY_OR_DEBUG_ASSERT(size >= 0) {
                size = 0;
            }
        }

        return static_cast<T2>(size);
    }
};

} // namespace mixxx
