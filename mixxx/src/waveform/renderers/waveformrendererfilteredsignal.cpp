#include "waveformrendererfilteredsignal.h"

#include "waveformwidgetrenderer.h"
#include "waveform/waveform.h"

#include "widget/wskincolor.h"
#include "trackinfoobject.h"
#include "widget/wwidget.h"

#include "defs.h"

WaveformRendererFilteredSignal::WaveformRendererFilteredSignal(
        WaveformWidgetRenderer* waveformWidgetRenderer)
    : WaveformRendererAbstract( waveformWidgetRenderer) {
}

WaveformRendererFilteredSignal::~WaveformRendererFilteredSignal() {
}

void WaveformRendererFilteredSignal::init() {
}

void WaveformRendererFilteredSignal::onResize() {
    m_lowLines.resize(m_waveformRenderer->getWidth());
    m_midLines.resize(m_waveformRenderer->getWidth());
    m_highLines.resize(m_waveformRenderer->getWidth());
}

void WaveformRendererFilteredSignal::setup(const QDomNode& node) {
    m_signalColor.setNamedColor(
                WWidget::selectNodeQString(node, "SignalColor"));
    m_signalColor = WSkinColor::getCorrectColor(m_signalColor);

    // TODO(vRince): fetch color from skin
    int h, s, l;
    m_signalColor.getHsl(&h, &s, &l);
    m_lowColor = QColor::fromHsl(h, s, 50, 128);
    m_midColor = QColor::fromHsl(h-2, s, 100, 128);
    m_highColor =  QColor::fromHsl(h+2, s, 200, 128);
}

void WaveformRendererFilteredSignal::draw(QPainter* painter,
                                          QPaintEvent* /*event*/) {
    const TrackPointer trackInfo = m_waveformRenderer->getTrackInfo();
    if (!trackInfo) {
        return;
    }

    const Waveform* waveform = trackInfo->getWaveform();
    if (waveform == NULL) {
        return;
    }

    int dataSize = waveform->getDataSize();
    if (dataSize <= 1) {
        return;
    }

    const WaveformData* data = waveform->data();
    if (data == NULL) {
        return;
    }

    const double xOffset = 0.5;

    int samplesPerPixel = m_waveformRenderer->getVisualSamplePerPixel();
    int numberOfSamples = m_waveformRenderer->getWidth() * samplesPerPixel;

    int currentPosition = 0;

    //TODO (vRince) not really accurate since waveform size une visual reasampling and
    //have two mores samples to hold the complete visual data
    currentPosition = m_waveformRenderer->getPlayPos() * dataSize;
    m_waveformRenderer->regulateVisualSample(currentPosition);

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing, false);
    painter->setRenderHints(QPainter::HighQualityAntialiasing, false);
    painter->setRenderHints(QPainter::SmoothPixmapTransform, false);
    painter->setWorldMatrixEnabled(false);

    const float halfHeight = m_waveformRenderer->getHeight()/2.0;
    const float heightFactor = halfHeight/255.0;

    for (int i = 0; i < numberOfSamples; i += samplesPerPixel) {
        const int xPos = i/samplesPerPixel;
        const int visualIndex = currentPosition + 2*i - numberOfSamples;
        if (visualIndex >= 0 && (visualIndex+1) < dataSize) {
            unsigned char maxLow[2] = {0, 0};
            unsigned char maxMid[2] = {0, 0};
            unsigned char maxHigh[2] = {0, 0};

            for (int subIndex = 0;
                 subIndex < 2*samplesPerPixel &&
                 visualIndex + subIndex + 1 < dataSize;
                 subIndex += 2) {
                const WaveformData& waveformData = *(data + visualIndex + subIndex);
                const WaveformData& waveformDataNext = *(data + visualIndex + subIndex + 1);
                maxLow[0] = math_max(maxLow[0], waveformData.filtered.low);
                maxLow[1] = math_max(maxLow[1], waveformDataNext.filtered.low);
                maxMid[0] = math_max(maxMid[0], waveformData.filtered.mid);
                maxMid[1] = math_max(maxMid[1], waveformDataNext.filtered.mid);
                maxHigh[0] = math_max(maxHigh[0], waveformData.filtered.high);
                maxHigh[1] = math_max(maxHigh[1], waveformDataNext.filtered.high);
            }

            m_lowLines[xPos].setLine(xPos, (int)(halfHeight-heightFactor*(float)maxLow[0]),
                                     xPos, (int)(halfHeight+heightFactor*(float)maxLow[1])+1);
            m_midLines[xPos].setLine(xPos, (int)(halfHeight-heightFactor*(float)maxMid[0]),
                                     xPos, (int)(halfHeight+heightFactor*(float)maxMid[1]));
            m_highLines[xPos].setLine(xPos, (int)(halfHeight-heightFactor*(float)maxHigh[0]),
                                      xPos, (int)(halfHeight+heightFactor*(float)maxHigh[1]));
        } else {
            m_lowLines[xPos].setLine(xPos, halfHeight, xPos, halfHeight+1);
            m_midLines[xPos].setLine(xPos, halfHeight, xPos, halfHeight);
            m_highLines[xPos].setLine(xPos, halfHeight, xPos, halfHeight);
        }
    }

    painter->setPen(QPen(QBrush(m_lowColor), 1));
    painter->drawLines(&m_lowLines[0], m_lowLines.size());
    painter->setPen(QPen(QBrush(m_midColor), 1));
    painter->drawLines(&m_midLines[0], m_midLines.size());
    painter->setPen(QPen(QBrush(m_highColor), 1));
    painter->drawLines(&m_highLines[0], m_highLines.size());

    painter->restore();
}
