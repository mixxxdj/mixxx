#include "waveformoverviewrenderer.h"

#include <QPainter>
#include <algorithm>
#include <cmath>

#include "util/colorcomponents.h"
#include "util/math.h"
#include "util/timer.h"
#include "waveform/renderers/waveformsignalcolors.h"

namespace {

// Height per band relative to the waveform data, fitted so that the overview
// matches the average of the RGB 3-band scrolling waveform.
constexpr float kRgb3BandGain[3] = {1.15f, 1.86f, 0.50f};

float meanSquare(unsigned char left, unsigned char right) {
    return (static_cast<float>(left) * left + static_cast<float>(right) * right) / 2.0f;
}

} // namespace

namespace waveformOverviewRenderer {

QImage render(ConstWaveformPointer pWaveform,
        mixxx::OverviewType type,
        const WaveformSignalColors& signalColors,
        bool mono) {
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
                signalColors,
                mono);
    } else if (type == mixxx::OverviewType::RGB3Band) {
        drawWaveformPartRGB3Band(&painter,
                pWaveform,
                nullptr,
                dataSize,
                signalColors,
                mono);
    } else if (type == mixxx::OverviewType::Filtered) {
        drawWaveformPartLMH(&painter,
                pWaveform,
                nullptr,
                dataSize,
                signalColors,
                mono);
    } else {
        drawWaveformPartRGB(&painter,
                pWaveform,
                nullptr,
                dataSize,
                signalColors,
                mono);
    }

    // Evaluate waveform ratio peak
    float peak = 1;
    for (int i = 0; i < dataSize; i += 2) {
        peak = math_max3(
                peak,
                static_cast<float>(pWaveform->getAll(i)),
                static_cast<float>(pWaveform->getAll(i + 1)));
    }
    // Normalize
    float diffGain = 0;
    if (peak > 1) {
        diffGain = 255 - peak - 1;
    }

    const int topLeft = static_cast<int>(mono ? diffGain * 2 : diffGain);
    const QRect sourceRect(0,
            topLeft,
            image.width(),
            image.height() -
                    2 * static_cast<int>(diffGain));
    QImage croppedImage = image.copy(sourceRect);
    // Copy image, otherwise QPainter crashes when we alter it.
    QImage normImage = croppedImage.scaled(image.size(),
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation);

    return normImage;
}

void drawWaveformPartRGB(
        QPainter* pPainter,
        ConstWaveformPointer pWaveform,
        int* start,
        int end,
        const WaveformSignalColors& signalColors,
        bool mono) {
    ScopedTimer t(QStringLiteral("waveformOverviewRenderer::drawNextPixmapPartRGB"));
    int startVal = 0;
    if (start) {
        startVal = *start;
    }

    const QColor lowColor = signalColors.getRgbLowColor();
    const QColor midColor = signalColors.getRgbMidColor();
    const QColor highColor = signalColors.getRgbHighColor();
    QColor color;

    float lowColor_r = 0, lowColor_g = 0, lowColor_b = 0,
          midColor_r = 0, midColor_g = 0, midColor_b = 0,
          highColor_r = 0, highColor_g = 0, highColor_b = 0,
          all = 0, low = 0, mid = 0, high = 0,
          red = 0, green = 0, blue = 0, max = 0;

    getRgbF(lowColor, &lowColor_r, &lowColor_g, &lowColor_b);
    getRgbF(midColor, &midColor_r, &midColor_g, &midColor_b);
    getRgbF(highColor, &highColor_r, &highColor_g, &highColor_b);

    if (mono) {
        // Mono means we're going to paint from bottom to top with l+r.
        const qreal dy = pPainter->deviceTransform().dy();
        pPainter->resetTransform();
        // shift y0 to bottom
        pPainter->translate(0, 2 * dy);
        // flip y-axis
        pPainter->scale(1, -1);
        for (int i = startVal, x = startVal / 2; i < end; i += 2, ++x) {
            // Left
            all = pWaveform->getAll(i) + pWaveform->getAll(i + 1);
            low = pWaveform->getLow(i) + pWaveform->getLow(i + 1);
            mid = pWaveform->getMid(i) + pWaveform->getMid(i + 1);
            high = pWaveform->getHigh(i) + pWaveform->getHigh(i + 1);

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
                pPainter->drawLine(x, static_cast<int>(all), x, 0);
            }
        }
    } else { // stereo
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
    }

    if (start) {
        *start = end;
    }
}

void drawWaveformPartLMH(
        QPainter* pPainter,
        ConstWaveformPointer pWaveform,
        int* start,
        int end,
        const WaveformSignalColors& signalColors,
        bool mono) {
    ScopedTimer t(QStringLiteral("waveformOverviewRenderer::drawNextPixmapPartLMH"));
    const QColor lowColor = signalColors.getLowColor();
    const QColor midColor = signalColors.getMidColor();
    const QColor highColor = signalColors.getHighColor();
    int startVal = 0;
    if (start) {
        startVal = *start;
    }

    if (mono) {
        // Mono means we're going to paint from bottom to top with l+r.
        const qreal dy = pPainter->deviceTransform().dy();
        pPainter->resetTransform();
        // shift y0 to bottom
        pPainter->translate(0, 2 * dy);
        // flip y-axis
        pPainter->scale(1, -1);

        for (int i = startVal, x = startVal / 2; i < end; i += 2, ++x) {
            x = i / 2;
            pPainter->setPen(lowColor);
            pPainter->drawLine(QPoint(x, 0),
                    QPoint(x, pWaveform->getLow(i) + pWaveform->getLow(i + 1)));

            pPainter->setPen(midColor);
            pPainter->drawLine(QPoint(x, 0),
                    QPoint(x, pWaveform->getMid(i) + pWaveform->getMid(i + 1)));

            pPainter->setPen(highColor);
            pPainter->drawLine(QPoint(x, 0),
                    QPoint(x, pWaveform->getHigh(i) + pWaveform->getHigh(i + 1)));
        }
    } else { // stereo
        for (int i = startVal, x = startVal / 2; i < end; i += 2, ++x) {
            x = i / 2;
            pPainter->setPen(lowColor);
            pPainter->drawLine(QPoint(x, -pWaveform->getLow(i)),
                    QPoint(x, pWaveform->getLow(i + 1)));

            pPainter->setPen(midColor);
            pPainter->drawLine(QPoint(x, -pWaveform->getMid(i)),
                    QPoint(x, pWaveform->getMid(i + 1)));

            pPainter->setPen(highColor);
            pPainter->drawLine(QPoint(x, -pWaveform->getHigh(i)),
                    QPoint(x, pWaveform->getHigh(i + 1)));
        }
    }

    if (start) {
        *start = end;
    }
}

RGB3BandHeights rgb3BandHeights(const Waveform& waveform, int index) {
    // Stereo-combined like the RGB 3-band scrolling waveform, where mid and
    // high together are drawn as the high band.
    const float low = meanSquare(waveform.getLow(index), waveform.getLow(index + 1));
    const float mid = meanSquare(waveform.getMid(index), waveform.getMid(index + 1));
    const float high = meanSquare(waveform.getHigh(index), waveform.getHigh(index + 1));
    RGB3BandHeights heights;
    heights.low = std::min(255.0f, kRgb3BandGain[0] * std::sqrt(low));
    heights.mid = std::min(255.0f, kRgb3BandGain[1] * std::sqrt(mid));
    heights.lowMid = std::min(heights.low, heights.mid);
    heights.high = std::min(255.0f, kRgb3BandGain[2] * std::sqrt(mid + high));
    return heights;
}

void drawWaveformPartRGB3Band(
        QPainter* pPainter,
        ConstWaveformPointer pWaveform,
        int* start,
        int end,
        const WaveformSignalColors& signalColors,
        bool mono) {
    ScopedTimer t(QStringLiteral("waveformOverviewRenderer::drawNextPixmapPartRGB3Band"));
    const QColor colors[4] = {signalColors.getRgb3BandLowColor(),
            signalColors.getRgb3BandMidColor(),
            signalColors.getRgb3BandLowMidColor(),
            signalColors.getRgb3BandHighColor()};
    int startVal = 0;
    if (start) {
        startVal = *start;
    }

    if (mono) {
        // Mono means we're going to paint from bottom to top with l+r.
        const qreal dy = pPainter->deviceTransform().dy();
        pPainter->resetTransform();
        // shift y0 to bottom
        pPainter->translate(0, 2 * dy);
        // flip y-axis
        pPainter->scale(1, -1);
    }

    for (int i = startVal; i < end; i += 2) {
        const qreal x = i / 2;
        const RGB3BandHeights heights = rgb3BandHeights(*pWaveform, i);
        const float layers[4] = {heights.low, heights.mid, heights.lowMid, heights.high};
        for (int layer = 0; layer < 4; ++layer) {
            pPainter->setPen(colors[layer]);
            if (mono) {
                pPainter->drawLine(QPointF(x, 0), QPointF(x, 2 * layers[layer]));
            } else {
                pPainter->drawLine(QPointF(x, -layers[layer]), QPointF(x, layers[layer]));
            }
        }
    }

    if (start) {
        *start = end;
    }
}

void drawWaveformPartHSV(
        QPainter* pPainter,
        ConstWaveformPointer pWaveform,
        int* start,
        int end,
        const WaveformSignalColors& signalColors,
        bool mono) {
    ScopedTimer t(QStringLiteral("waveformOverviewRenderer::drawNextPixmapPartHSV"));
    int startVal = 0;
    if (start) {
        startVal = *start;
    }

    float h = 0, s = 0, v = 0, lo = 0, hi = 0, total = 0;
    // Get HSV of low color.
    const QColor lowColor = signalColors.getLowColor();
    getHsvF(lowColor, &h, &s, &v);
    QColor color;

    unsigned char low[2] = {0, 0};
    unsigned char high[2] = {0, 0};
    unsigned char mid[2] = {0, 0};
    unsigned char all[2] = {0, 0};

    if (mono) {
        // Mono means we're going to paint from bottom to top with l+r.
        const qreal dy = pPainter->deviceTransform().dy();
        pPainter->resetTransform();
        // shift y0 to bottom
        pPainter->translate(0, 2 * dy);
        // flip y-axis
        pPainter->scale(1, -1);
    }

    for (int i = startVal, x = startVal / 2; i < end; i += 2, ++x) {
        x = i / 2;
        all[0] = pWaveform->getAll(i);
        all[1] = pWaveform->getAll(i + 1);

        if (!all[0] && !all[1]) {
            continue;
        }

        low[0] = pWaveform->getLow(i);
        low[1] = pWaveform->getLow(i + 1);
        mid[0] = pWaveform->getMid(i);
        mid[1] = pWaveform->getMid(i + 1);
        high[0] = pWaveform->getHigh(i);
        high[1] = pWaveform->getHigh(i + 1);

        total = (low[0] + low[1] + mid[0] + mid[1] +
                        high[0] + high[1]) *
                1.2f;

        // Prevent division by zero
        if (total > 0) {
            // Normalize low and high
            // (mid not need, because it not change the color)
            lo = (low[0] + low[1]) / total;
            hi = (high[0] + high[1]) / total;
        } else {
            lo = hi = 0.0;
        }

        // Set color
        color.setHsvF(h, 1.0f - hi, 1.0f - lo);

        if (mono) {
            pPainter->setPen(color);
            pPainter->drawLine(QPoint(i / 2, 0),
                    QPoint(i / 2, all[0] + all[1]));
        } else {
            pPainter->setPen(color);
            pPainter->drawLine(QPoint(i / 2, -all[0]),
                    QPoint(i / 2, all[1]));
        }
    }

    if (start) {
        *start = end;
    }
}

} // namespace waveformOverviewRenderer
