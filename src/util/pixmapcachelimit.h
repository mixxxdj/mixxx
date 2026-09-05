#pragma once

#include <limits>

#include "util/fpclassify.h"

namespace mixxx::pixmapcache {

constexpr int kCacheLimitAtOneXKiB = 32 * 1024;

inline int cacheLimitKiB(double devicePixelRatio) {
    if (!util_isfinite(devicePixelRatio) || devicePixelRatio <= 0.0) {
        devicePixelRatio = 1.0;
    }
    const double scaledLimit =
            kCacheLimitAtOneXKiB * devicePixelRatio * devicePixelRatio;
    if (scaledLimit >= std::numeric_limits<int>::max()) {
        return std::numeric_limits<int>::max();
    }
    return static_cast<int>(scaledLimit);
}

} // namespace mixxx::pixmapcache
