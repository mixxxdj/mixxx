#include "waveformrendererfilteredsignal.h"

#include "waveformwidgetrenderer.h"
#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"

#include "widget/wskincolor.h"
#include "trackinfoobject.h"
#include "widget/wwidget.h"

#include "defs.h"

#include "controlobjectthreadmain.h"

WaveformRendererFilteredSignal::WaveformRendererFilteredSignal(
        WaveformWidgetRenderer* waveformWidgetRenderer)
    : WaveformRendererSignalBase( waveformWidgetRenderer) {
}

WaveformRendererFilteredSignal::~WaveformRendererFilteredSignal() {
}

void WaveformRendererFilteredSignal::onInit() {
}

void WaveformRendererFilteredSignal::onResize() {
    m_lowLines.resize(m_waveformRenderer->getWidth());
    m_midLines.resize(m_waveformRenderer->getWidth());
    m_highLines.resize(m_waveformRenderer->getWidth());
}

void WaveformRendererFilteredSignal::onSetup(const QDomNode& node) {

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

    const int dataSize = waveform->getDataSize();
    if (dataSize <= 1) {
        return;
    }

    const WaveformData* data = waveform->data();
    if (data == NULL) {
        return;
    }

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
    painter->resetTransform();

    // Per-band gain from the EQ knobs.
    float lowGain(1.0), midGain(1.0), highGain(1.0), allGain(1.0);
    if (m_lowFilterControlObject &&
            m_midFilterControlObject &&
            m_highFilterControlObject) {
        lowGain = m_lowFilterControlObject->get();
        midGain = m_midFilterControlObject->get();
        highGain = m_highFilterControlObject->get();
    }
    allGain = m_waveformRenderer->getGain();

    WaveformWidgetFactory* factory = WaveformWidgetFactory::instance();
    allGain *= factory->getVisualGain(::WaveformWidgetFactory::All);
    lowGain *= factory->getVisualGain(WaveformWidgetFactory::Low);
    midGain *= factory->getVisualGain(WaveformWidgetFactory::Mid);
    highGain *= factory->getVisualGain(WaveformWidgetFactory::High);

    const float halfHeight = (float)m_waveformRenderer->getHeight()/2.0;

    const float heightFactor = m_alignment == Qt::AlignCenter
            ? allGain*halfHeight/255.0
            : allGain*m_waveformRenderer->getHeight()/255.0;

    //draw reference line
    if( m_alignment == Qt::AlignCenter) {
        painter->setPen(m_axesColor);
        painter->drawLine(0,halfHeight,m_waveformRenderer->getWidth(),halfHeight);
    }

    int actualLowLineNumber = 0;
    int actualMidLineNumber = 0;
    int actualHighLineNumber = 0;

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

            if( maxLow[0] && maxLow[1]) {
                switch( m_alignment) {
                case Qt::AlignBottom :
                    m_lowLines[actualLowLineNumber].setLine(
                                xPos, m_waveformRenderer->getHeight(),
                                xPos, m_waveformRenderer->getHeight() - (int)(heightFactor*lowGain*(float)math_max(maxLow[0],maxLow[1])));
                    break;
                case Qt::AlignTop :
                    m_lowLines[actualLowLineNumber].setLine(
                                xPos, 0,
                                xPos, (int)(heightFactor*lowGain*(float)math_max(maxLow[0],maxLow[1])));
                    break;
                default :
                    m_lowLines[actualLowLineNumber].setLine(
                                xPos, (int)(halfHeight-heightFactor*(float)maxLow[0]*lowGain),
                                xPos, (int)(halfHeight+heightFactor*(float)maxLow[1]*lowGain));
                    break;
                }
                actualLowLineNumber++;
            }
            if( maxMid[0] && maxMid[1]) {
                switch( m_alignment) {
                case Qt::AlignBottom :
                    m_midLines[actualMidLineNumber].setLine(
                                xPos, m_waveformRenderer->getHeight(),
                                xPos, m_waveformRenderer->getHeight() - (int)(heightFactor*midGain*(float)math_max(maxMid[0],maxMid[1])));
                    break;
                case Qt::AlignTop :
                    m_midLines[actualMidLineNumber].setLine(
                                xPos, 0,
                                xPos, (int)(heightFactor*midGain*(float)math_max(maxMid[0],maxMid[1])));
                    break;
                default :
                    m_midLines[actualMidLineNumber].setLine(
                                xPos, (int)(halfHeight-heightFactor*(float)maxMid[0]*midGain),
                                xPos, (int)(halfHeight+heightFactor*(float)maxMid[1]*midGain));
                    break;
                }
                actualMidLineNumber++;
            }
            if( maxHigh[0] && maxHigh[1]) {
                switch( m_alignment) {
                case Qt::AlignBottom :
                    m_highLines[actualHighLineNumber].setLine(
                                xPos, m_waveformRenderer->getHeight(),
                                xPos, m_waveformRenderer->getHeight() - (int)(heightFactor*highGain*(float)math_max(maxHigh[0],maxHigh[1])));
                    break;
                case Qt::AlignTop :
                    m_highLines[actualHighLineNumber].setLine(
                                xPos, 0,
                                xPos, (int)(heightFactor*highGain*(float)math_max(maxHigh[0],maxHigh[1])));
                    break;
                default :
                    m_highLines[actualHighLineNumber].setLine(
                                xPos, (int)(halfHeight-heightFactor*(float)maxHigh[0]*highGain),
                                xPos, (int)(halfHeight+heightFactor*(float)maxHigh[1]*highGain));
                    break;
                }
                actualHighLineNumber++;
            }
        }
    }

    painter->setPen(QPen(QBrush(m_colors.getLowColor()), 1));
    painter->drawLines(&m_lowLines[0], actualLowLineNumber);
    painter->setPen(QPen(QBrush(m_colors.getMidColor()), 1));
    painter->drawLines(&m_midLines[0], actualMidLineNumber);
    painter->setPen(QPen(QBrush(m_colors.getHighColor()), 1));
    painter->drawLines(&m_highLines[0], actualHighLineNumber);

    painter->restore();
}
