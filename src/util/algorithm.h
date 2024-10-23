#pragma once

#include <algorithm>
#include <iterator>
#include <type_traits>
#include <utility>

#include "util/assert.h"

namespace mixxx {

/**
 * @brief replace the value at `location` with `value` in the range [begin, end)
 * while keeping it sorted Behavior is undefined if `*evict`, `*newPosition` or
 * `evict <= newPosition` is undefined.
 *
 * @tparam It
 * @tparam T
 * @param begin begin of the range to search in for `value`
 * @param end
 * @param location
 * @param value
 * @return new position of `value`
 */
template<typename It, typename T>
constexpr It replace_sorted(T&& value, It newPosition, It evict) {
    static_assert(std::is_convertible_v<std::remove_cvref_t<T>,
                          typename std::iterator_traits<It>::value_type>,
            "can't assign value to underlying iterator");

    // the range is now likely not sorted anymore.
    // There are two ways to restore order.
    // in both cases, we want the value stored
    // at `evict` to be at `newPosition`
    // while retaining the order of the elements in between.
    if (newPosition <= evict) {
        // -------+-----+-------+-----
        //   ...  | ... | evict | ...
        // -------+-----+-------+-----
        //        ^new
        std::move_backward(newPosition, evict, evict + 1);
    } else {
        // -----+-------+-----+-----
        //  ... | evict | ... | ...
        // -----+-------+-----+-----
        //                    ^new
        std::move(evict + 1, newPosition, evict);
        --newPosition;
    }
    *newPosition = std::forward<T>(value);
    return newPosition;
}

template<typename It, typename T>
constexpr It replace_sorted(It begin, It end, T&& value, It evict) {
    DEBUG_ASSERT(std::is_sorted(begin, end));
    DEBUG_ASSERT(begin <= evict && evict < end);
    auto newPosition = std::lower_bound(begin, end, value);
    auto it = replace_sorted(std::forward<T>(value), newPosition, evict);
    DEBUG_ASSERT(std::is_sorted(begin, end));
    return it;
}

} // namespace mixxx
