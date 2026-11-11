#include "waveform/renderers/rgb3bandlevels.h"

#include <algorithm>
#include <array>

namespace {

constexpr int kBinCount = 256;
constexpr float kMinNormalization = 0.4f;
constexpr float kMaxNormalization = 3.0f;

int amplitudeBin(unsigned char left, unsigned char right) {
    return static_cast<int>(std::lround(rgb3band::combinedAmplitude(left, right)));
}

} // namespace

namespace rgb3band {

void bandLevels(const Waveform& waveform,
        int size,
        const float quantile[3],
        float level[3]) {
    const int frameCount = std::clamp(size, 0, waveform.getDataSize()) / 2;
    std::array<std::array<int, kBinCount>, 3> histogram{};
    const WaveformData* data = waveform.data();
    for (int frame = 0; frame < frameCount; ++frame) {
        const WaveformFilteredData& left = data[2 * frame].filtered;
        const WaveformFilteredData& right = data[2 * frame + 1].filtered;
        ++histogram[0][amplitudeBin(left.low, right.low)];
        ++histogram[1][amplitudeBin(left.mid, right.mid)];
        ++histogram[2][amplitudeBin(left.high, right.high)];
    }

    for (int band = 0; band < 3; ++band) {
        const float target = quantile[band] * frameCount;
        int count = 0;
        int bin = 0;
        for (; bin < kBinCount - 1; ++bin) {
            count += histogram[band][bin];
            if (count >= target) {
                break;
            }
        }
        level[band] = static_cast<float>(std::max(bin, 1)) / (kBinCount - 1);
    }
}

float normalization(float level, float gain, float slope) {
    return std::clamp(gain / std::pow(level, slope), kMinNormalization, kMaxNormalization);
}

} // namespace rgb3band
