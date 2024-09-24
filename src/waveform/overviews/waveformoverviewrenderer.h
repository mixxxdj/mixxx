#pragma once

#include <QColor>

#include "waveform/waveform.h"
#include "widget/woverview.h"

class QPainter;
class WaveformSignalColors;

class WaveformOverviewRenderer {
  public:
    static QImage render(ConstWaveformPointer,
            WOverview::Type type,
            const WaveformSignalColors& signalColors);
    static void drawWaveformPartRGB(
            QPainter* pPainter,
            ConstWaveformPointer pWaveform,
            int* start,
            int end,
            const WaveformSignalColors& signalColors);
    static void drawWaveformPartLMH(
            QPainter* pPainter,
            ConstWaveformPointer pWaveform,
            int* start,
            int end,
            const WaveformSignalColors& signalColors);
    static void drawWaveformPartHSV(
            QPainter* pPainter,
            ConstWaveformPointer pWaveform,
            int* start,
            int end,
            const WaveformSignalColors& signalColors);
};
