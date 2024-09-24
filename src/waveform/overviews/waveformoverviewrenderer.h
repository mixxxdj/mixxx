#pragma once

#include <QColor>

#include "waveform/overviews/overviewtype.h"
#include "waveform/waveform.h"

class QPainter;

class WaveformOverviewRenderer {
  public:
    static QImage render(ConstWaveformPointer, mixxx::OverviewType type);
    static void drawWaveformPartRGB(
            QPainter* pPainter,
            ConstWaveformPointer pWaveform,
            int* start,
            int end,
            QColor rgbLowColor,
            QColor rgbMidColor,
            QColor rgbHighColor);
    static void drawWaveformPartLMH(
            QPainter* pPainter,
            ConstWaveformPointer pWaveform,
            int* start,
            int end,
            QColor lowColor,
            QColor midColor,
            QColor highColor);
    static void drawWaveformPartHSV(
            QPainter* pPainter,
            ConstWaveformPointer pWaveform,
            int* start,
            int end,
            QColor lowColor);

    static void setRgbLowColor(QColor rgbLow);
    static void setRgbMidColor(QColor color);
    static void setRgbHighColor(QColor color);
    static void setFilteredLowColor(QColor color);
    static void setFilteredMidColor(QColor color);
    static void setFilteredHighColor(QColor color);
    static void setSignalColor(QColor color);

    static constexpr QColor defaultLowColor = QColor(255, 0, 0);
    static constexpr QColor defaultMidColor = QColor(0, 255, 0);
    static constexpr QColor defaultHighColor = QColor(0, 0, 255);

    static constexpr QColor defaultSignalColor = QColor(200, 200, 200);

    static QColor s_rgbLowColor;
    static QColor s_rgbMidColor;
    static QColor s_rgbHighColor;

    static QColor s_filteredLowColor;
    static QColor s_filteredMidColor;
    static QColor s_filteredHighColor;

    static QColor s_signalColor;
};
