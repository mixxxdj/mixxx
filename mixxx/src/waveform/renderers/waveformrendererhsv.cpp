#include "waveformrendererhsv.h"

#include "waveformwidgetrenderer.h"
#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"

#include "widget/wskincolor.h"
#include "trackinfoobject.h"
#include "widget/wwidget.h"

#include "defs.h"

#include "controlobjectthreadmain.h"

WaveformRendererHSV::WaveformRendererHSV(
        WaveformWidgetRenderer* waveformWidgetRenderer)
    : WaveformRendererSignalBase( waveformWidgetRenderer) {
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

    const Waveform* waveform = trackInfo->getWaveform();
    if (waveform == NULL) {
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

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing, false);
    painter->setRenderHints(QPainter::HighQualityAntialiasing, false);
    painter->setRenderHints(QPainter::SmoothPixmapTransform, false);
    painter->setWorldMatrixEnabled(false);
    painter->resetTransform();

    const double firstVisualIndex = m_waveformRenderer->getFirstDisplayedPosition() * dataSize;
    const double lastVisualIndex = m_waveformRenderer->getLastDisplayedPosition() * dataSize;

    const double offset = firstVisualIndex;

    // Represents the # of waveform data points per horizontal pixel.
    const double gain = (lastVisualIndex - firstVisualIndex) /
            (double)m_waveformRenderer->getWidth();

    float allGain(1.0);
    allGain = m_waveformRenderer->getGain();

    WaveformWidgetFactory* factory = WaveformWidgetFactory::instance();
    allGain *= factory->getVisualGain(::WaveformWidgetFactory::All);

    // Save HSV of waveform color
    double h,s,v;

    // Get base color of waveform in the HSV format (s and v isn't use)
    m_pColors->getLowColor().getHsvF(&h,&s,&v);

    QColor color;
    float lo, hi, total;

    const float halfHeight = (float)m_waveformRenderer->getHeight()/2.0;

    const float heightFactor = allGain*halfHeight/255.0;

    //draw reference line
    painter->setPen(m_axesColor);
    painter->drawLine(0,halfHeight,m_waveformRenderer->getWidth(),halfHeight);

    for (int x = 0; x < m_waveformRenderer->getWidth(); ++x) {
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
        visualFrameStart = math_max(math_min(lastVisualFrame, visualFrameStart), 0);
        visualFrameStop = math_max(math_min(lastVisualFrame, visualFrameStop), 0);

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
            maxAll[0] = math_max( maxAll[0], (int)waveformData.filtered.all);
            maxAll[1] = math_max( maxAll[1], (int)waveformDataNext.filtered.all);
        }

        if( maxAll[0] && maxAll[1] ) {
            // Calculate sum, to normalize
            // Also multiply on 1.2 to prevent very dark or light color
            total = (maxLow[0] + maxLow[1] + maxMid[0] + maxMid[1] + maxHigh[0] + maxHigh[1]) * 1.2;

            // prevent division by zero
            if( total > 0 )
            {
                // Normalize low and high (mid not need, because it not change the color)
                lo = (maxLow[0] + maxLow[1]) / total;
                hi = (maxHigh[0] + maxHigh[1]) / total;
            }
            else
                lo = hi = 0.0;

            // Set color
            color.setHsvF(h, 1.0-hi, 1.0-lo);

            painter->setPen(color);
            switch (m_alignment) {
                case Qt::AlignBottom :
                    painter->drawLine(
                        x, m_waveformRenderer->getHeight(),
                        x, m_waveformRenderer->getHeight() - (int)(heightFactor*(float)math_max(maxAll[0],maxAll[1])));
                    break;
                case Qt::AlignTop :
                    painter->drawLine(
                        x, 0,
                        x, (int)(heightFactor*(float)math_max(maxAll[0],maxAll[1])));
                    break;
                default :
                    painter->drawLine(
                        x, (int)(halfHeight-heightFactor*(float)maxAll[0]),
                        x, (int)(halfHeight+heightFactor*(float)maxAll[1]));
            }
        }
    }

    painter->restore();
}
