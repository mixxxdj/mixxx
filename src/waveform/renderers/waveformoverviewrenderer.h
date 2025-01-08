#pragma once

#include <QColor>

#include "waveform/overviewtype.h"
#include "waveform/waveform.h"

class QPainter;
class WaveformSignalColors;

class WaveformOverviewRenderer {
  public:
    static QImage render(ConstWaveformPointer pWaveform,
            mixxx::OverviewType type,
            const WaveformSignalColors& signalColors,
            bool mono = false);
    /// These paint methods allow "mono" rendering (mono-mixdown, bottom-aligned).
    /// Note: Don't use with WOverview, it's not adjusted yet! It does some
    /// additional scaling for normalization which will atm cut off the bottom part.
    static void drawWaveformPartRGB(
            QPainter* pPainter,
            ConstWaveformPointer pWaveform,
            int* start,
            int end,
            const WaveformSignalColors& signalColors,
            bool mono = false);
    static void drawWaveformPartLMH(
            QPainter* pPainter,
            ConstWaveformPointer pWaveform,
            int* start,
            int end,
            const WaveformSignalColors& signalColors,
            bool mono = false);
    static void drawWaveformPartHSV(
            QPainter* pPainter,
            ConstWaveformPointer pWaveform,
            int* start,
            int end,
            const WaveformSignalColors& signalColors,
            bool mono = false);
};
