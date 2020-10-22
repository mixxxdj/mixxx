#include "waveformrendererrgb.h"

#include "waveformwidgetrenderer.h"
#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"
#include "widget/wskincolor.h"
#include "track/track.h"
#include "widget/wwidget.h"
#include "util/math.h"
#include "util/painterscope.h"

WaveformRendererRGB::WaveformRendererRGB(
        WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRendererSignalBase(waveformWidgetRenderer) {
}

WaveformRendererRGB::~WaveformRendererRGB() {
}

void WaveformRendererRGB::onSetup(const QDomNode& /* node */) {
}

void WaveformRendererRGB::draw(QPainter* painter,
                                          QPaintEvent* /*event*/) {
    const TrackPointer trackInfo = m_waveformRenderer->getTrackInfo();
    if (!trackInfo) {
        return;
    }

    ConstWaveformPointer waveform = trackInfo->getWaveform();
    if (waveform.isNull()) {
        return;
    }

    const int dataSize = waveform->getDataSize();
    if (dataSize <= 1) {
        return;
    }

    const WaveformData* data = waveform->data();
    if (data == NULL) {
        return;
    }

    PainterScope PainterScope(painter);

    painter->setRenderHints(QPainter::Antialiasing, false);
    painter->setRenderHints(QPainter::SmoothPixmapTransform, false);
    painter->setWorldMatrixEnabled(false);
    painter->resetTransform();

    // Rotate if drawing vertical waveforms
    if (m_waveformRenderer->getOrientation() == Qt::Vertical) {
        painter->setTransform(QTransform(0, 1, 1, 0, 0, 0));
    }

    const double firstVisualIndex = m_waveformRenderer->getFirstDisplayedPosition() * dataSize;
    const double lastVisualIndex = m_waveformRenderer->getLastDisplayedPosition() * dataSize;

    const double offset = firstVisualIndex;

    // Represents the # of waveform data points per horizontal pixel.
    const double gain = (lastVisualIndex - firstVisualIndex) /
            (double)m_waveformRenderer->getLength();

    // Per-band gain from the EQ knobs.
    float allGain(1.0), lowGain(1.0), midGain(1.0), highGain(1.0);
    getGains(&allGain, &lowGain, &midGain, &highGain);

    QColor color;

    QPen pen;
    pen.setCapStyle(Qt::FlatCap);
    pen.setWidthF(math_max(1.0, 1.0 / m_waveformRenderer->getVisualSamplePerPixel()));

    const int breadth = m_waveformRenderer->getBreadth();
    const float halfBreadth = static_cast<float>(breadth) / 2.0f;

    const float heightFactor = allGain * halfBreadth / sqrtf(255 * 255 * 3);

    // Draw reference line
    painter->setPen(m_pColors->getAxesColor());
    painter->drawLine(QLineF(0, halfBreadth, m_waveformRenderer->getLength(), halfBreadth));

    for (int x = 0; x < m_waveformRenderer->getLength(); ++x) {
        // Width of the x position in visual indices.
        const double xSampleWidth = gain * x;

        // Effective visual index of x
        const double xVisualSampleIndex = xSampleWidth + offset;

        // Our current pixel (x) corresponds to a number of visual samples
        // (visualSamplerPerPixel) in our waveform object. We take the max of
        // all the data points on either side of xVisualSampleIndex within a
        // window of 'maxSamplingRange' visual samples to measure the maximum
        // data point contained by this pixel.
        double maxSamplingRange = gain / 2.0;

        // Since xVisualSampleIndex is in visual-samples (e.g. R,L,R,L) we want
        // to check +/- maxSamplingRange frames, not samples. To do this, divide
        // xVisualSampleIndex by 2. Since frames indices are integers, we round
        // to the nearest integer by adding 0.5 before casting to int.
        int visualFrameStart = int(xVisualSampleIndex / 2.0 - maxSamplingRange + 0.5);
        int visualFrameStop = int(xVisualSampleIndex / 2.0 + maxSamplingRange + 0.5);
        const int lastVisualFrame = dataSize / 2 - 1;

        // We now know that some subset of [visualFrameStart, visualFrameStop]
        // lies within the valid range of visual frames. Clamp
        // visualFrameStart/Stop to within [0, lastVisualFrame].
        visualFrameStart = math_clamp(visualFrameStart, 0, lastVisualFrame);
        visualFrameStop = math_clamp(visualFrameStop, 0, lastVisualFrame);

        int visualIndexStart = visualFrameStart * 2;
        int visualIndexStop  = visualFrameStop * 2;

        unsigned char maxLow  = 0;
        unsigned char maxMid  = 0;
        unsigned char maxHigh = 0;
        float maxAll = 0.;
        float maxAllNext = 0.;

        for (int i = visualIndexStart;
             i >= 0 && i + 1 < dataSize && i + 1 <= visualIndexStop; i += 2) {
            const WaveformData& waveformData = data[i];
            const WaveformData& waveformDataNext = data[i + 1];

            maxLow  = math_max3(maxLow,  waveformData.filtered.low,  waveformDataNext.filtered.low);
            maxMid  = math_max3(maxMid,  waveformData.filtered.mid,  waveformDataNext.filtered.mid);
            maxHigh = math_max3(maxHigh, waveformData.filtered.high, waveformDataNext.filtered.high);
            float all = static_cast<float>(pow(waveformData.filtered.low * lowGain, 2) +
                    pow(waveformData.filtered.mid * midGain, 2) +
                    pow(waveformData.filtered.high * highGain, 2));
            maxAll = math_max(maxAll, all);
            float allNext = static_cast<float>(pow(waveformDataNext.filtered.low * lowGain, 2) +
                    pow(waveformDataNext.filtered.mid * midGain, 2) +
                    pow(waveformDataNext.filtered.high * highGain, 2));
            maxAllNext = math_max(maxAllNext, allNext);
        }

        qreal maxLowF = maxLow * lowGain;
        qreal maxMidF = maxMid * midGain;
        qreal maxHighF = maxHigh * highGain;

        qreal red   = maxLowF * m_rgbLowColor_r + maxMidF * m_rgbMidColor_r + maxHighF * m_rgbHighColor_r;
        qreal green = maxLowF * m_rgbLowColor_g + maxMidF * m_rgbMidColor_g + maxHighF * m_rgbHighColor_g;
        qreal blue  = maxLowF * m_rgbLowColor_b + maxMidF * m_rgbMidColor_b + maxHighF * m_rgbHighColor_b;

        // Compute maximum (needed for value normalization)
        qreal max = math_max3(red, green, blue);

        // Prevent division by zero
        if (max > 0.0f) {
            // Set color
            color.setRgbF(red / max, green / max, blue / max);

            pen.setColor(color);

            painter->setPen(pen);
            switch (m_alignment) {
                case Qt::AlignBottom:
                case Qt::AlignRight:
                    painter->drawLine(
                        x, breadth,
                        x, breadth - (int)(heightFactor * sqrtf(math_max(maxAll, maxAllNext))));
                    break;
                case Qt::AlignTop:
                case Qt::AlignLeft:
                    painter->drawLine(
                        x, 0,
                        x, (int)(heightFactor * sqrtf(math_max(maxAll, maxAllNext))));
                    break;
                default:
                    painter->drawLine(
                        x, (int)(halfBreadth - heightFactor * sqrtf(maxAll)),
                        x, (int)(halfBreadth + heightFactor * sqrtf(maxAllNext)));
            }
        }
    }
}
