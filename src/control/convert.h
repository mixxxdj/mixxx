#pragma once

#include "util/color/rgbcolor.h"
#include "util/assert.h"

namespace mixxx {

namespace control {

typedef double value_t;

constexpr value_t kInvalidRgbColor = -1.0;

inline value_t valueFromRgbColor(RgbColor color) {
    return static_cast<RgbColor::code_t>(color);
}

inline value_t valueFromRgbColor(RgbColor::optional_t color) {
    if (color) {
        return valueFromRgbColor(*color);
    } else {
        return kInvalidRgbColor;
    }
}

inline RgbColor::optional_t valueToRgbColor(value_t value) {
    if (value >= 0) {
        const auto code = static_cast<RgbColor::code_t>(value);
        // If value is out of range then unused bits will be
        // discarded when converting into an RgbColor. Nevertheless
        // check with a debug assertion that the given value
        // is in range as expected. Out of range values indicate
        // programming errors in other components.
        DEBUG_ASSERT(RgbColor::isValidCode(code));
        return RgbColor::optional(code);
    } else {
        // < 0 or NaN (if non-signalling)
        DEBUG_ASSERT(value == kInvalidRgbColor);
        return RgbColor::nullopt();
    }
}

} // namespace control

} // namespace mixxx
