#pragma once

#include <cmath>

#include "waveform/waveform.h"

/// Helpers shared by the RGB 3-band scrolling and overview waveforms.
///
/// Like Rekordbox, the RGB 3-band waveforms scale each band per track, so
/// quiet masters and tracks with little bass are not drawn tiny. The scale is
/// derived from the band's level, a high quantile of its amplitude over the
/// whole track.
namespace rgb3band {

/// Stereo-combined amplitude of one visual frame, in the units of the
/// waveform data (0-255)
inline float combinedAmplitude(unsigned char left, unsigned char right) {
    return std::sqrt((static_cast<float>(left) * left +
                             static_cast<float>(right) * right) /
            2.0f);
}

/// Calculates the level (0-1) of the low, mid, and high band as the given
/// quantile of the stereo-combined amplitude over the first `size` data
/// elements.
void bandLevels(const Waveform& waveform,
        int size,
        const float quantile[3],
        float level[3]);

/// Returns gain / level^slope, limited for silent or clipping tracks.
float normalization(float level, float gain, float slope);

} // namespace rgb3band
