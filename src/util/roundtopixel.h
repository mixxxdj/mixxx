#pragma once

#include <cmath>

inline auto createFunctionRoundToPixel(qreal devicePixelRatio) {
    return [devicePixelRatio](qreal pos) {
        return std::round(pos * devicePixelRatio) / devicePixelRatio;
    };
}
