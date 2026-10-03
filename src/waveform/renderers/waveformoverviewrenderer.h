#pragma once

#include <QColor>

#include "waveform/overviewtype.h"
#include "waveform/waveform.h"

class QPainter;
class WaveformSignalColors;

namespace waveformOverviewRenderer {
/// Layer heights of the RGB 3-band overview for one stereo sample, in the
/// units of the waveform data (0-255 per channel).
struct RGB3BandHeights {
    float low;
    float mid;
    float lowMid;
    float high;
};
/// Per-track scale of the low, mid, and high band, calculated from the first
/// `size` data elements of the waveform summary.
struct RGB3BandNormalization {
    float band[3];
};
RGB3BandNormalization rgb3BandNormalization(const Waveform& waveform, int size);
RGB3BandHeights rgb3BandHeights(const Waveform& waveform,
        int index,
        const RGB3BandNormalization& normalization);

/// This returns the normalized fullsize image
/// for the library's overview column.
QImage render(ConstWaveformPointer pWaveform,
        mixxx::OverviewType type,
        const WaveformSignalColors& signalColors,
        bool mono = false);

/// These paint methods return the fullsize image
/// They allow "mono" rendering (mono-mixdown, bottom-aligned).
/// Note: Don't use mono = true with WOverview, it's not adjusted yet! It does some
/// additional scaling for normalization which will atm cut off the bottom part.
void drawWaveformPartRGB(
        QPainter* pPainter,
        ConstWaveformPointer pWaveform,
        int* start,
        int end,
        const WaveformSignalColors& signalColors,
        bool mono = false);
void drawWaveformPartLMH(
        QPainter* pPainter,
        ConstWaveformPointer pWaveform,
        int* start,
        int end,
        const WaveformSignalColors& signalColors,
        bool mono = false);
void drawWaveformPartRGB3Band(
        QPainter* pPainter,
        ConstWaveformPointer pWaveform,
        int* start,
        int end,
        const WaveformSignalColors& signalColors,
        bool mono = false);
void drawWaveformPartHSV(
        QPainter* pPainter,
        ConstWaveformPointer pWaveform,
        int* start,
        int end,
        const WaveformSignalColors& signalColors,
        bool mono = false);
} // namespace waveformOverviewRenderer
