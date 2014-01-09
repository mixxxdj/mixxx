#include "waveformrendererrgb.h"

#include "waveformwidgetrenderer.h"
#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"

#include "widget/wskincolor.h"
#include "trackinfoobject.h"
#include "widget/wwidget.h"

#include "defs.h"

#include "controlobjectthreadmain.h"

#define MAX3(a, b, c)  ((a) > (b) ? ((a) > (c) ? (a) : (c)) : ((b) > (c) ? (b) : (c)))

WaveformRendererRGB::WaveformRendererRGB(
        WaveformWidgetRenderer* waveformWidgetRenderer)
    : WaveformRendererSignalBase( waveformWidgetRenderer) {
}

WaveformRendererRGB::~WaveformRendererRGB() {
}

void WaveformRendererRGB::onSetup(const QDomNode& node) {
    Q_UNUSED(node);

    m_lowColor = m_pColors->getLowColor();
    m_midColor = m_pColors->getMidColor();
    m_highColor = m_pColors->getHighColor();
}

void WaveformRendererRGB::draw(QPainter* painter,
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

    QColor color;

    const float halfHeight = (float)m_waveformRenderer->getHeight()/2.0;

    const float heightFactor = allGain*halfHeight/255.0;

    // Draw reference line
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
        int visualIndexStop  = visualFrameStop * 2;

        unsigned char maxLow  = 0;
        unsigned char maxMid  = 0;
        unsigned char maxHigh = 0;
        unsigned char maxAllA = 0;
        unsigned char maxAllB = 0;

        for (int i = visualIndexStart;
             i >= 0 && i + 1 < dataSize && i + 1 <= visualIndexStop; i += 2) {
            const WaveformData& waveformData = *(data + i);
            const WaveformData& waveformDataNext = *(data + i + 1);

            maxLow  = MAX3(maxLow,  waveformData.filtered.low,  waveformDataNext.filtered.low);
            maxMid  = MAX3(maxMid,  waveformData.filtered.mid,  waveformDataNext.filtered.mid);
            maxHigh = MAX3(maxHigh, waveformData.filtered.high, waveformDataNext.filtered.high);
            maxAllA = math_max(maxAllA, waveformData.filtered.all);
            maxAllB = math_max(maxAllB, waveformDataNext.filtered.all);
        }

        // Compute sum (needed for value normalization)
        float sum = maxLow + maxMid + maxHigh;

        // Prevent division by zero
        if (sum > 0.0f) {
            // Normalize low, mid and high values
            float lo = (float) maxLow  / sum;
            float mi = (float) maxMid  / sum;
            float hi = (float) maxHigh / sum;

            float red   = lo * m_lowColor.redF()   + mi * m_midColor.redF()   + hi * m_highColor.redF();
            float green = lo * m_lowColor.greenF() + mi * m_midColor.greenF() + hi * m_highColor.greenF();
            float blue  = lo * m_lowColor.blueF()  + mi * m_midColor.blueF()  + hi * m_highColor.blueF();

            // Set color
            color.setRgbF(red, green, blue);

            painter->setPen(color);
            switch (m_alignment) {
                case Qt::AlignBottom :
                    painter->drawLine(
                        x, m_waveformRenderer->getHeight(),
                        x, m_waveformRenderer->getHeight() - (int)(heightFactor*(float)math_max(maxAllA,maxAllB)));
                    break;
                case Qt::AlignTop :
                    painter->drawLine(
                        x, 0,
                        x, (int)(heightFactor*(float)math_max(maxAllA,maxAllB)));
                    break;
                default :
                    painter->drawLine(
                        x, (int)(halfHeight-heightFactor*(float)maxAllA),
                        x, (int)(halfHeight+heightFactor*(float)maxAllB));
            }
        }
    }

    painter->restore();
}
