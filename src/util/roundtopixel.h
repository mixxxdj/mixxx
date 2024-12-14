#pragma once

#include <cmath>

inline auto makeRoundToPixel(float devicePixelRatio) {
    return [devicePixelRatio](float pos) {
        return std::round(pos * devicePixelRatio) / devicePixelRatio;
    };
}
