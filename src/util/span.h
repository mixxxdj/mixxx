#pragma once

#include <span>

namespace mixxx {

/// The class offers a group of utilities (methods) for working with std::span.
class SpanUtil {
  public:
    /// The method creates std::span from pointer and size.
    /// In most cases, the pointer to the raw data of a data structure
    /// is used, and the size of the data structure.
    template<typename T, typename S>
    static std::span<T> spanFromPtrLen(T* ptr, S size) {
        return std::span<T>{ptr, static_cast<typename std::span<T>::size_type>(size)};
    }

  private:
    // TODO(davidchocholaty) conversion method for SINT with appropriate checking and casting
};

} // namespace mixxx
