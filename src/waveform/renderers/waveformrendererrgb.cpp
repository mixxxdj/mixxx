#include "waveformrendererrgb.h"

#include "waveformwidgetrenderer.h"
#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"

#include "widget/wskincolor.h"
#include "trackinfoobject.h"
#include "widget/wwidget.h"
#include "util/math.h"

WaveformRendererRGB::WaveformRendererRGB(
        WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRendererSignalBase(waveformWidgetRenderer) {
}

WaveformRendererRGB::~WaveformRendererRGB() {
}

void WaveformRendererRGB::onSetup(const QDomNode& /* node */) {
}

void WaveformRendererRGB::setup(const QDomNode& node,
                                       const SkinContext& context) {
    WaveformRendererSignalBase::setup(node, context);

    m_lowColor.setNamedColor(context.selectString(node, "SignalLowColor"));
    if (!m_lowColor.isValid()) {
        m_lowColor.setRgb(255,0,0);
    }
    m_lowColor  = WSkinColor::getCorrectColor(m_lowColor);

    m_midColor.setNamedColor(context.selectString(node, "SignalMidColor"));
    if (!m_midColor.isValid()) {
        m_midColor.setRgb(0,255,0);
    }
    m_midColor  = WSkinColor::getCorrectColor(m_midColor);

    m_highColor.setNamedColor(context.selectString(node, "SignalHighColor"));
    if (!m_highColor.isValid()) {
        m_highColor.setRgb(0,0,255);
    }
    m_highColor = WSkinColor::getCorrectColor(m_highColor);
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
    getGains(&allGain, NULL, NULL, NULL);

    QColor color;

    const float halfHeight = (float)m_waveformRenderer->getHeight()/2.0;

    const float heightFactor = allGain*halfHeight/255.0;

    // Draw reference line
    painter->setPen(m_pColors->getAxesColor());
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
        visualFrameStart = math_clamp(visualFrameStart, 0, lastVisualFrame);
        visualFrameStop = math_clamp(visualFrameStop, 0, lastVisualFrame);

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

            maxLow  = math_max3(maxLow,  waveformData.filtered.low,  waveformDataNext.filtered.low);
            maxMid  = math_max3(maxMid,  waveformData.filtered.mid,  waveformDataNext.filtered.mid);
            maxHigh = math_max3(maxHigh, waveformData.filtered.high, waveformDataNext.filtered.high);
            maxAllA = math_max(maxAllA, waveformData.filtered.all);
            maxAllB = math_max(maxAllB, waveformDataNext.filtered.all);
        }

        int red   = maxLow * m_lowColor.red()   + maxMid * m_midColor.red()   + maxHigh * m_highColor.red();
        int green = maxLow * m_lowColor.green() + maxMid * m_midColor.green() + maxHigh * m_highColor.green();
        int blue  = maxLow * m_lowColor.blue()  + maxMid * m_midColor.blue()  + maxHigh * m_highColor.blue();

        // Compute maximum (needed for value normalization)
        float max = (float) math_max3(red, green, blue);

        // Prevent division by zero
        if (max > 0.0f) {
            // Set color
            color.setRgbF(red / max, green / max, blue / max);

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
