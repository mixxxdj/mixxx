#pragma once

#include <QColor>

#include "util/singleton.h"
#include "waveform/waveform.h"
#include "widget/woverview.h"

class QPainter;

class WaveformOverviewRenderer : public Singleton<WaveformOverviewRenderer> {
  public:
    QImage render(ConstWaveformPointer, WOverview::Type type);
    void drawWaveformPartRGB(
            QPainter* pPainter,
            ConstWaveformPointer pWaveform,
            int* start,
            int end,
            QColor lowColor = Qt::red,
            QColor midColor = Qt::green,
            QColor highColor = Qt::blue);
    void drawWaveformPartLMH(
            QPainter* pPainter,
            ConstWaveformPointer pWaveform,
            int* start,
            int end,
            QColor lowColor = Qt::red,
            QColor midColor = Qt::green,
            QColor highColor = Qt::blue);
    void drawWaveformPartHSV(
            QPainter* pPainter,
            ConstWaveformPointer pWaveform,
            int* start,
            int end,
            QColor lowColor = Qt::lightGray);
};
