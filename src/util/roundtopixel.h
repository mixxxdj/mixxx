#pragma once

#include <cmath>

inline auto createFunctionRoundToPixel(qreal devicePixelRatio) {
    return [devicePixelRatio](qreal pos) {
        return std::round(pos * devicePixelRatio) / devicePixelRatio;
    };
}

inline auto createFunctionRoundToFloatPixel(float devicePixelRatio) {
    return [devicePixelRatio](float pos) {
        return std::round(pos * devicePixelRatio) / devicePixelRatio;
    };
}
