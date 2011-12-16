#include "waveformrendererfilteredsignal.h"

#include "waveformwidgetrenderer.h"
#include "widget/wskincolor.h"
#include "trackinfoobject.h"
#include "widget/wwidget.h"
#include "waveform.h"
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
    qDebug() << "WaveformRendererFilteredSignal::onResize";
    m_lowLines.reserve(2*m_waveformRenderer->getWidth());
    m_midLines.reserve(2*m_waveformRenderer->getWidth());
    m_highLines.reserve(2*m_waveformRenderer->getWidth());
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
    const TrackInfoObject* trackInfo = m_waveformRenderer->getTrackInfo().data();

    if (!trackInfo) {
        return;
    }

    const Waveform* waveform = trackInfo->getWaveForm();
    const QVector<unsigned char>& waveformData = waveform->getConstData();

    // TODO(rryan): clear() actually clears the memory we have reserve()'d. This
    // causes memory thrashing.
    m_lowLines.clear();
    m_midLines.clear();
    m_highLines.clear();

    int samplesPerPixel = m_waveformRenderer->getZoomFactor();
    samplesPerPixel = math_min(2, samplesPerPixel);
    int numberOfSamples = m_waveformRenderer->getWidth() * samplesPerPixel;

    int currentPosition = 0;
    if (m_waveformRenderer->getPlayPos() >= 0) {
        currentPosition = static_cast<int>(m_waveformRenderer->getPlayPos()*
                                           waveformData.size());
        currentPosition -= (currentPosition % (2 * samplesPerPixel));
    }

    painter->save();
    painter->setWorldMatrixEnabled(false);

    float halfHeight = m_waveformRenderer->getHeight()/2.0;
    float heightFactor = halfHeight/255.0;

    const QVector<unsigned char>& lowData = waveform->getConstLowData();
    const QVector<unsigned char>& midData = waveform->getConstMidData();
    const QVector<unsigned char>& highData = waveform->getConstHighData();

    for (int i = 0; i < numberOfSamples; i += 2*samplesPerPixel) {
        int xPos = i/samplesPerPixel;
        int thisIndex = currentPosition + 2*i - numberOfSamples;
        if (thisIndex >= 0 && (thisIndex+1) < waveformData.size()) {
            unsigned char maxLow[2] = {0, 0};
            unsigned char maxBand[2] = {0, 0};
            unsigned char maxHigh[2] = {0, 0};

            for (int sampleIndex = 0; sampleIndex < 2*samplesPerPixel;
                 ++sampleIndex) {
                // TODO(rryan) QVector::operator[] will show very high in
                // profiling runs due to the bounds checking. Better to call
                // constData() on the QVector first.
                maxLow[0] = math_max(
                    maxLow[0], lowData[thisIndex+sampleIndex]);
                maxLow[1] = math_max(
                    maxLow[1], lowData[thisIndex+sampleIndex+1]);
                maxBand[0] = math_max(
                    maxBand[0], midData[thisIndex+sampleIndex]);
                maxBand[1] = math_max(
                    maxBand[1], midData[thisIndex+sampleIndex+1]);
                maxHigh[0] = math_max(
                    maxHigh[0], highData[thisIndex+sampleIndex]);
                maxHigh[1] = math_max(
                    maxHigh[1], highData[thisIndex+sampleIndex+1]);
            }

            // TODO(rryan) push_back is allocating memory due to the clear()
            // above.
            m_lowLines.push_back(
                QLineF(xPos, (int)(halfHeight-heightFactor*(float)maxLow[0]),
                       xPos, (int)(halfHeight+heightFactor*(float)maxLow[1])));
            m_midLines.push_back(
                QLine(xPos, (int)(halfHeight-heightFactor*(float)maxBand[0]),
                      xPos, (int)(halfHeight+heightFactor*(float)maxBand[1])));
            m_highLines.push_back(
                QLine(xPos, (int)(halfHeight-heightFactor*(float)maxHigh[0]),
                      xPos, (int)(halfHeight+heightFactor*(float)maxHigh[1])));
        } else {
            m_lowLines.push_back(QLine(xPos, 0, xPos, 0));
            m_midLines.push_back(QLine(xPos, 0, xPos, 0));
            m_highLines.push_back(QLine(xPos, 0, xPos, 0));
        }
    }

    // TODO(rryan) calling data() on a QVector clones it. This causes intense
    // memory thrashing and skips on low-end computers.
    painter->setPen(QPen(QBrush(m_lowColor), 2));
    painter->drawLines(m_lowLines.data(), m_lowLines.size());
    painter->setPen(QPen(QBrush(m_midColor), 2));
    painter->drawLines(m_midLines.data(), m_midLines.size());
    painter->setPen(QPen(QBrush(m_highColor), 2));
    painter->drawLines(m_highLines.data(), m_highLines.size());
    painter->restore();
}
