#pragma once

#include <QColor>

#include "util/singleton.h"
#include "waveform/waveform.h"
#include "widget/woverview.h"

class QPainter;
class WaveformSignalColors;

class WaveformOverviewRenderer : public Singleton<WaveformOverviewRenderer> {
  public:
    QImage render(ConstWaveformPointer,
            WOverview::Type type,
            const WaveformSignalColors& signalColors);
    void drawWaveformPartRGB(
            QPainter* pPainter,
            ConstWaveformPointer pWaveform,
            int* start,
            int end,
            const WaveformSignalColors& signalColors);
    void drawWaveformPartLMH(
            QPainter* pPainter,
            ConstWaveformPointer pWaveform,
            int* start,
            int end,
            const WaveformSignalColors& signalColors);
    void drawWaveformPartHSV(
            QPainter* pPainter,
            ConstWaveformPointer pWaveform,
            int* start,
            int end,
            const WaveformSignalColors& signalColors);
};
