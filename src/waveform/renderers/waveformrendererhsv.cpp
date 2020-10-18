#include "waveformrendererhsv.h"

#include "waveformwidgetrenderer.h"
#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"
#include "widget/wskincolor.h"
#include "track/track.h"
#include "widget/wwidget.h"
#include "util/math.h"
#include "util/painterscope.h"

WaveformRendererHSV::WaveformRendererHSV(
        WaveformWidgetRenderer* waveformWidgetRenderer)
    : WaveformRendererSignalBase(waveformWidgetRenderer) {
}

WaveformRendererHSV::~WaveformRendererHSV() {
}

void WaveformRendererHSV::onSetup(const QDomNode& node) {
    Q_UNUSED(node);
}

void WaveformRendererHSV::draw(QPainter* painter,
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

    float allGain(1.0);
    getGains(&allGain, NULL, NULL, NULL);

    // Save HSV of waveform color. NOTE(rryan): On ARM, qreal is float so it's
    // important we use qreal here and not double or float or else we will get
    // build failures on ARM.
    qreal h, s, v;

    // Get base color of waveform in the HSV format (s and v isn't use)
    m_pColors->getLowColor().getHsvF(&h, &s, &v);

    QColor color;
    float lo, hi, total;

    QPen pen;
    pen.setCapStyle(Qt::FlatCap);
    pen.setWidth(math_max(1.0, 1.0 / m_waveformRenderer->getVisualSamplePerPixel()));

    const int breadth = m_waveformRenderer->getBreadth();
    const float halfBreadth = (float)breadth / 2.0;

    const float heightFactor = allGain * halfBreadth / 255.0;

    //draw reference line
    painter->setPen(m_pColors->getAxesColor());
    painter->drawLine(0, halfBreadth, m_waveformRenderer->getLength(), halfBreadth);

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
        int visualIndexStop = visualFrameStop * 2;

        int maxLow[2] = {0, 0};
        int maxHigh[2] = {0, 0};
        int maxMid[2] = {0, 0};
        int maxAll[2] = {0, 0};

        for (int i = visualIndexStart;
             i >= 0 && i + 1 < dataSize && i + 1 <= visualIndexStop; i += 2) {
            const WaveformData& waveformData = *(data + i);
            const WaveformData& waveformDataNext = *(data + i + 1);
            maxLow[0] = math_max(maxLow[0], (int)waveformData.filtered.low);
            maxLow[1] = math_max(maxLow[1], (int)waveformDataNext.filtered.low);
            maxMid[0] = math_max(maxMid[0], (int)waveformData.filtered.mid);
            maxMid[1] = math_max(maxMid[1], (int)waveformDataNext.filtered.mid);
            maxHigh[0] = math_max(maxHigh[0], (int)waveformData.filtered.high);
            maxHigh[1] = math_max(maxHigh[1], (int)waveformDataNext.filtered.high);
            maxAll[0] = math_max(maxAll[0], (int)waveformData.filtered.all);
            maxAll[1] = math_max(maxAll[1], (int)waveformDataNext.filtered.all);
        }

        if (maxAll[0] && maxAll[1]) {
            // Calculate sum, to normalize
            // Also multiply on 1.2 to prevent very dark or light color
            total = (maxLow[0] + maxLow[1] + maxMid[0] + maxMid[1] + maxHigh[0] + maxHigh[1]) * 1.2;

            // prevent division by zero
            if (total > 0)
            {
                // Normalize low and high (mid not need, because it not change the color)
                lo = (maxLow[0] + maxLow[1]) / total;
                hi = (maxHigh[0] + maxHigh[1]) / total;
            }
            else
                lo = hi = 0.0;

            // Set color
            color.setHsvF(h, 1.0-hi, 1.0-lo);

            pen.setColor(color);

            painter->setPen(pen);
            switch (m_alignment) {
                case Qt::AlignBottom :
                case Qt::AlignRight :
                    painter->drawLine(
                        x, breadth,
                        x, breadth - (int)(heightFactor * (float)math_max(maxAll[0],maxAll[1])));
                    break;
                case Qt::AlignTop :
                case Qt::AlignLeft :
                    painter->drawLine(
                        x, 0,
                        x, (int)(heightFactor * (float)math_max(maxAll[0],maxAll[1])));
                    break;
                default :
                    painter->drawLine(
                        x, (int)(halfBreadth - heightFactor * (float)maxAll[0]),
                        x, (int)(halfBreadth + heightFactor * (float)maxAll[1]));
            }
        }
    }
}
