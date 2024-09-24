#pragma once

#include <QColor>

#include "waveform/overviews/overviewtype.h"
#include "waveform/waveform.h"

class QPainter;
class WaveformSignalColors;

class WaveformOverviewRenderer {
  public:
    static QImage render(ConstWaveformPointer,
            mixxx::OverviewType type,
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
