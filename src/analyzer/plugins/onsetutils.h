#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>
#include <vector>

#include "util/assert.h"

// Small helpers shared by refineConstantBeatGrid() and the half
// beat model.
namespace mixxx::onsetutils {

// Resolution of a folded onset histogram, in bins per onset frame.
constexpr int kBinsPerFrame = 4;

// Circular smoothing of a folded onset histogram, +-0.75 onset frames at
// kBinsPerFrame bins per frame.
constexpr std::array<double, 7> kFoldKernel = {1, 2, 3, 4, 3, 2, 1};
constexpr double kFoldKernelSum = std::accumulate(kFoldKernel.begin(), kFoldKernel.end(), 0.0);

// A position in frames in onset frames: onset frame i is centred on
// i * stepSizeFrames + stepSizeFrames / 2.
inline double onsetFramePosition(double position, int stepSizeFrames) {
    return (position - stepSizeFrames / 2.0) / stepSizeFrames;
}

// The onset frame nearest to a position in frames.
inline long onsetFrame(double position, int stepSizeFrames) {
    return std::lround(onsetFramePosition(position, stepSizeFrames));
}

// i wrapped into [0, count).
inline int wrapIndex(int i, int count) {
    return (i % count + count) % count;
}

// Bin bin of a circular histogram of count bins smoothed with kFoldKernel.
inline double foldKernelSmoothed(const double* pBins, int count, int bin) {
    constexpr int kHalfKernel = static_cast<int>(kFoldKernel.size()) / 2;
    double smoothed = 0;
    for (int k = -kHalfKernel; k <= kHalfKernel; ++k) {
        smoothed += kFoldKernel[k + kHalfKernel] * pBins[wrapIndex(bin + k, count)];
    }
    return smoothed;
}

inline double median(std::vector<double> values) {
    DEBUG_ASSERT(!values.empty());
    const auto middle = values.begin() + values.size() / 2;
    std::nth_element(values.begin(), middle, values.end());
    if (values.size() % 2 == 1) {
        return *middle;
    }
    return (*std::max_element(values.begin(), middle) + *middle) / 2;
}

// x wrapped into [0, period).
inline double wrap(double x, double period) {
    const double wrapped = x - period * std::floor(x / period);
    // With fast math the subtraction can be slightly below 0 or reach period.
    return wrapped < 0 || wrapped >= period ? 0 : wrapped;
}

// x wrapped into [-period / 2, period / 2].
inline double wrapCentered(double x, double period) {
    return x - period * std::round(x / period);
}

// Log magnitude of a band, the level the band flux is taken from.
inline double bandLevel(float magnitude) {
    return std::log1p(10.0 * magnitude);
}

} // namespace mixxx::onsetutils
