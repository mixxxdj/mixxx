#pragma once

#include <cmath>
#include <concepts>

template<typename T>
    requires std::floating_point<T>
inline auto createFunctionRoundToPixel(T devicePixelRatio) {
    return [devicePixelRatio](T pos) -> T {
        return std::round(pos * devicePixelRatio) / devicePixelRatio;
    };
}
