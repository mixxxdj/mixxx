#include "waveformrendererfilteredsignal.h"

#include "waveformwidgetrenderer.h"
#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"
#include "control/controlproxy.h"
#include "widget/wskincolor.h"
#include "track/track.h"
#include "widget/wwidget.h"
#include "util/math.h"
#include "util/painterscope.h"

WaveformRendererFilteredSignal::WaveformRendererFilteredSignal(
        WaveformWidgetRenderer* waveformWidgetRenderer)
    : WaveformRendererSignalBase(waveformWidgetRenderer) {
}

WaveformRendererFilteredSignal::~WaveformRendererFilteredSignal() {
}

void WaveformRendererFilteredSignal::onResize() {
    m_lowLines.resize(m_waveformRenderer->getLength());
    m_midLines.resize(m_waveformRenderer->getLength());
    m_highLines.resize(m_waveformRenderer->getLength());
}

void WaveformRendererFilteredSignal::onSetup(const QDomNode& node) {
    Q_UNUSED(node);
}

void WaveformRendererFilteredSignal::draw(QPainter* painter,
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

    // Represents the # of waveform data points per horizontal pixel.
    const double gain = (lastVisualIndex - firstVisualIndex) /
            (double)m_waveformRenderer->getLength();

    // Per-band gain from the EQ knobs.
    float allGain(1.0), lowGain(1.0), midGain(1.0), highGain(1.0);
    getGains(&allGain, &lowGain, &midGain, &highGain);

    const float breadth = m_waveformRenderer->getBreadth();
    const float halfBreadth = breadth / 2.0;

    const float heightFactor = m_alignment == Qt::AlignCenter
            ? allGain*halfBreadth/255.0
            : allGain*m_waveformRenderer->getBreadth()/255.0;

    //draw reference line
    if (m_alignment == Qt::AlignCenter) {
        painter->setPen(m_pColors->getAxesColor());
        painter->drawLine(0, halfBreadth, m_waveformRenderer->getLength(), halfBreadth);
    }

    int actualLowLineNumber = 0;
    int actualMidLineNumber = 0;
    int actualHighLineNumber = 0;

    for (int x = 0; x < m_waveformRenderer->getLength(); ++x) {
        // Width of the x position in visual indices.
        const double xSampleWidth = gain * x;

        // Effective visual index of x
        const double xVisualSampleIndex = xSampleWidth + firstVisualIndex;

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

        // If the entire sample range is off the screen then don't calculate a
        // point for this pixel.
        const int lastVisualFrame = dataSize / 2 - 1;
        if (visualFrameStop < 0 || visualFrameStart > lastVisualFrame) {
            continue;
        }

        // We now know that some subset of [visualFrameStart, visualFrameStop]
        // lies within the valid range of visual frames. Clamp
        // visualFrameStart/Stop to within [0, lastVisualFrame].
        visualFrameStart = math_clamp(visualFrameStart, 0, lastVisualFrame);
        visualFrameStop = math_clamp(visualFrameStop, 0, lastVisualFrame);

        int visualIndexStart = visualFrameStart * 2;
        int visualIndexStop = visualFrameStop * 2;

        // if (x == m_waveformRenderer->getLength() / 2) {
        //     qDebug() << "audioVisualRatio" << waveform->getAudioVisualRatio();
        //     qDebug() << "visualSampleRate" << waveform->getVisualSampleRate();
        //     qDebug() << "audioSamplesPerVisualPixel" << waveform->getAudioSamplesPerVisualSample();
        //     qDebug() << "visualSamplePerPixel" << visualSamplePerPixel;
        //     qDebug() << "xSampleWidth" << xSampleWidth;
        //     qDebug() << "xVisualSampleIndex" << xVisualSampleIndex;
        //     qDebug() << "maxSamplingRange" << maxSamplingRange;;
        //     qDebug() << "Sampling pixel " << x << "over [" << visualIndexStart << visualIndexStop << "]";
        // }

        unsigned char maxLow[2] = {0, 0};
        unsigned char maxMid[2] = {0, 0};
        unsigned char maxHigh[2] = {0, 0};

        for (int i = visualIndexStart;
             i >= 0 && i + 1 < dataSize && i + 1 <= visualIndexStop; i += 2) {
            const WaveformData& waveformData = *(data + i);
            const WaveformData& waveformDataNext = *(data + i + 1);
            maxLow[0] = math_max(maxLow[0], waveformData.filtered.low);
            maxLow[1] = math_max(maxLow[1], waveformDataNext.filtered.low);
            maxMid[0] = math_max(maxMid[0], waveformData.filtered.mid);
            maxMid[1] = math_max(maxMid[1], waveformDataNext.filtered.mid);
            maxHigh[0] = math_max(maxHigh[0], waveformData.filtered.high);
            maxHigh[1] = math_max(maxHigh[1], waveformDataNext.filtered.high);
        }

        if (maxLow[0] && maxLow[1]) {
            switch (m_alignment) {
                case Qt::AlignBottom :
                case Qt::AlignRight :
                    m_lowLines[actualLowLineNumber].setLine(
                        x, breadth,
                        x, breadth - (int)(heightFactor*lowGain*(float)math_max(maxLow[0],maxLow[1])));
                    break;
                case Qt::AlignTop :
                case Qt::AlignLeft :
                    m_lowLines[actualLowLineNumber].setLine(
                        x, 0,
                        x, (int)(heightFactor*lowGain*(float)math_max(maxLow[0],maxLow[1])));
                    break;
                default :
                    m_lowLines[actualLowLineNumber].setLine(
                        x, (int)(halfBreadth-heightFactor*(float)maxLow[0]*lowGain),
                        x, (int)(halfBreadth+heightFactor*(float)maxLow[1]*lowGain));
                    break;
            }
            actualLowLineNumber++;
        }
        if (maxMid[0] && maxMid[1]) {
            switch (m_alignment) {
                case Qt::AlignBottom :
                case Qt::AlignRight :
                    m_midLines[actualMidLineNumber].setLine(
                        x, breadth,
                        x, breadth - (int)(heightFactor*midGain*(float)math_max(maxMid[0],maxMid[1])));
                    break;
                case Qt::AlignTop :
                case Qt::AlignLeft :
                    m_midLines[actualMidLineNumber].setLine(
                        x, 0,
                        x, (int)(heightFactor*midGain*(float)math_max(maxMid[0],maxMid[1])));
                    break;
                default :
                    m_midLines[actualMidLineNumber].setLine(
                        x, (int)(halfBreadth-heightFactor*(float)maxMid[0]*midGain),
                        x, (int)(halfBreadth+heightFactor*(float)maxMid[1]*midGain));
                    break;
            }
            actualMidLineNumber++;
        }
        if (maxHigh[0] && maxHigh[1]) {
            switch (m_alignment) {
                case Qt::AlignBottom :
                case Qt::AlignRight :
                    m_highLines[actualHighLineNumber].setLine(
                        x, breadth,
                        x, breadth - (int)(heightFactor*highGain*(float)math_max(maxHigh[0],maxHigh[1])));
                    break;
                case Qt::AlignTop :
                case Qt::AlignLeft :
                    m_highLines[actualHighLineNumber].setLine(
                        x, 0,
                        x, (int)(heightFactor*highGain*(float)math_max(maxHigh[0],maxHigh[1])));
                    break;
                default :
                    m_highLines[actualHighLineNumber].setLine(
                        x, (int)(halfBreadth-heightFactor*(float)maxHigh[0]*highGain),
                        x, (int)(halfBreadth+heightFactor*(float)maxHigh[1]*highGain));
                    break;
            }
            actualHighLineNumber++;
        }
    }

    double lineThickness = math_max(1.0, 1.0 / m_waveformRenderer->getVisualSamplePerPixel());

    painter->setPen(QPen(QBrush(m_pColors->getLowColor()), lineThickness, Qt::SolidLine, Qt::FlatCap));
    if (m_pLowKillControlObject && m_pLowKillControlObject->get() == 0.0) {
       painter->drawLines(&m_lowLines[0], actualLowLineNumber);
    }
    painter->setPen(QPen(QBrush(m_pColors->getMidColor()), lineThickness, Qt::SolidLine, Qt::FlatCap));
    if (m_pMidKillControlObject && m_pMidKillControlObject->get() == 0.0) {
        painter->drawLines(&m_midLines[0], actualMidLineNumber);
    }
    painter->setPen(QPen(QBrush(m_pColors->getHighColor()), lineThickness, Qt::SolidLine, Qt::FlatCap));
    if (m_pHighKillControlObject && m_pHighKillControlObject->get() == 0.0) {
        painter->drawLines(&m_highLines[0], actualHighLineNumber);
    }
}
