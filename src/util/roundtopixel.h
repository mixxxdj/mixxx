#pragma once

#include <cmath>

inline auto createFunctionRoundToPixel(float devicePixelRatio) {
    return [devicePixelRatio](float pos) {
        return std::round(pos * devicePixelRatio) / devicePixelRatio;
    };
}
