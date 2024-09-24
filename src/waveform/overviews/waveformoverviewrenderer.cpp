#include "waveformoverviewrenderer.h"

#include <QPainter>

#include "util/colorcomponents.h"
#include "util/math.h"
#include "waveform/renderers/waveformsignalcolors.h"

QImage WaveformOverviewRenderer::render(ConstWaveformPointer pWaveform,
        mixxx::OverviewType type,
        const WaveformSignalColors& signalColors) {
    const int dataSize = pWaveform->getDataSize();
    if (dataSize <= 0) {
        return QImage();
    }

    QImage image(dataSize / 2, 2 * 255, QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(0, 0, 0, 0).value());

    QPainter painter(&image);
    painter.translate(0.0, static_cast<double>(image.height()) / 2.0);

    if (type == mixxx::OverviewType::HSV) {
        drawWaveformPartHSV(&painter,
                pWaveform,
                nullptr,
                dataSize,
                signalColors);
    } else if (type == mixxx::OverviewType::Filtered) {
        drawWaveformPartLMH(&painter,
                pWaveform,
                nullptr,
                dataSize,
                signalColors);
    } else {
        drawWaveformPartRGB(&painter,
                pWaveform,
                nullptr,
                dataSize,
                signalColors);
    }

    return image;
}

void WaveformOverviewRenderer::drawWaveformPartRGB(
        QPainter* pPainter,
        ConstWaveformPointer pWaveform,
        int* start,
        int end,
        const WaveformSignalColors& signalColors) {
    int startVal = 0;
    if (start) {
        startVal = *start;
    }

    const QColor lowColor = signalColors.getRgbLowColor();
    const QColor midColor = signalColors.getRgbMidColor();
    const QColor highColor = signalColors.getRgbHighColor();

    // TODO initialize?
    float lowColor_r, lowColor_g, lowColor_b,
            midColor_r, midColor_g, midColor_b,
            highColor_r, highColor_g, highColor_b,
            all, low, mid, high, red, green, blue, max;
    QColor color;

    getRgbF(lowColor, &lowColor_r, &lowColor_g, &lowColor_b);
    getRgbF(midColor, &midColor_r, &midColor_g, &midColor_b);
    getRgbF(highColor, &highColor_r, &highColor_g, &highColor_b);

    for (int i = startVal, x = startVal / 2; i < end; i += 2, ++x) {
        // Left
        all = pWaveform->getAll(i);
        low = pWaveform->getLow(i);
        mid = pWaveform->getMid(i);
        high = pWaveform->getHigh(i);

        red = low * lowColor_r + mid * midColor_r + high * highColor_r;
        green = low * lowColor_g + mid * midColor_g + high * highColor_g;
        blue = low * lowColor_b + mid * midColor_b + high * highColor_b;
        // Normalize
        max = math_max3(red, green, blue);
        // Draw
        if (max > 0.0) {
            color.setRgbF(static_cast<float>(low / max),
                    static_cast<float>(mid / max),
                    static_cast<float>(high / max));
            pPainter->setPen(color);
            pPainter->drawLine(x, static_cast<int>(-all), x, 0);
        }

        // Right
        all = pWaveform->getAll(i + 1);
        low = pWaveform->getLow(i + 1);
        mid = pWaveform->getMid(i + 1);
        high = pWaveform->getHigh(i + 1);

        red = low * lowColor_r + mid * midColor_r + high * highColor_r;
        green = low * lowColor_g + mid * midColor_g + high * highColor_g;
        blue = low * lowColor_b + mid * midColor_b + high * highColor_b;

        max = math_max3(red, green, blue);

        if (max > 0.0) {
            color.setRgbF(static_cast<float>(low / max),
                    static_cast<float>(mid / max),
                    static_cast<float>(high / max));
            pPainter->setPen(color);
            pPainter->drawLine(x, 0, x, static_cast<int>(all));
        }
    }
    if (start) {
        *start = end;
    }
}

void WaveformOverviewRenderer::drawWaveformPartLMH(
        QPainter* pPainter,
        ConstWaveformPointer pWaveform,
        int* start,
        int end,
        const WaveformSignalColors& signalColors) {
    const QColor lowColor = signalColors.getLowColor();
    const QColor midColor = signalColors.getMidColor();
    const QColor highColor = signalColors.getHighColor();
    int startVal = 0;
    if (start) {
        startVal = *start;
    }

    for (int i = startVal, x = startVal / 2; i < end; i += 2, ++x) {
        x = i / 2;
        unsigned char low_r = pWaveform->getLow(i);
        unsigned char low_l = pWaveform->getLow(i + 1);
        if (low_l || low_r) {
            pPainter->setPen(lowColor);
            pPainter->drawLine(QPoint(x, -low_r), QPoint(x, low_l));
        }

        pPainter->setPen(midColor);
        pPainter->drawLine(QPoint(x, -pWaveform->getMid(i)),
                QPoint(x, pWaveform->getMid(i + 1)));

        pPainter->setPen(highColor);
        pPainter->drawLine(QPoint(x, -pWaveform->getHigh(i)),
                QPoint(x, pWaveform->getHigh(i + 1)));
    }
    if (start) {
        *start = end;
    }
}

void WaveformOverviewRenderer::drawWaveformPartHSV(
        QPainter* pPainter,
        ConstWaveformPointer pWaveform,
        int* start,
        int end,
        const WaveformSignalColors& signalColors) {
    int startVal = 0;
    if (start) {
        startVal = *start;
    }

    const QColor lowColor = signalColors.getLowColor();
    // Get HSV of low color.
    float h, s, v; // TODO initialize?
    getHsvF(lowColor, &h, &s, &v);

    QColor color;
    float lo, hi, total; // TODO initialize?

    unsigned char maxLow[2] = {0, 0};
    unsigned char maxHigh[2] = {0, 0};
    unsigned char maxMid[2] = {0, 0};
    unsigned char maxAll[2] = {0, 0};

    for (int i = startVal, x = startVal / 2; i < end; i += 2, ++x) {
        x = i / 2;
        maxAll[0] = pWaveform->getAll(i);
        maxAll[1] = pWaveform->getAll(i + 1);
        if (maxAll[0] || maxAll[1]) {
            maxLow[0] = pWaveform->getLow(i);
            maxLow[1] = pWaveform->getLow(i + 1);
            maxMid[0] = pWaveform->getMid(i);
            maxMid[1] = pWaveform->getMid(i + 1);
            maxHigh[0] = pWaveform->getHigh(i);
            maxHigh[1] = pWaveform->getHigh(i + 1);

            total = (maxLow[0] + maxLow[1] + maxMid[0] + maxMid[1] +
                            maxHigh[0] + maxHigh[1]) *
                    1.2f;

            // Prevent division by zero
            if (total > 0) {
                // Normalize low and high
                // (mid not need, because it not change the color)
                lo = (maxLow[0] + maxLow[1]) / total;
                hi = (maxHigh[0] + maxHigh[1]) / total;
            } else {
                lo = hi = 0.0;
            }

            // Set color
            color.setHsvF(h, 1.0f - hi, 1.0f - lo);

            pPainter->setPen(color);
            pPainter->drawLine(QPoint(i / 2, -maxAll[0]),
                    QPoint(i / 2, maxAll[1]));
        }
    }
    if (start) {
        *start = end;
    }
}
