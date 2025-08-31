#pragma once

#include <QColor>

#include "waveform/overviewtype.h"
#include "waveform/waveform.h"

class QPainter;
class WaveformSignalColors;

namespace waveformOverviewRenderer {
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
void drawWaveformPartHSV(
        QPainter* pPainter,
        ConstWaveformPointer pWaveform,
        int* start,
        int end,
        const WaveformSignalColors& signalColors,
        bool mono = false);
} // namespace waveformOverviewRenderer
