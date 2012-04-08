#include "glwaveformrenderersimplesignal.h"

#include "waveformwidgetrenderer.h"
#include "waveform/waveform.h"

#include "trackinfoobject.h"

#include <QLinearGradient>

GLWaveformRendererSimpleSignal::GLWaveformRendererSimpleSignal( WaveformWidgetRenderer* waveformWidgetRenderer) :
    WaveformRendererAbstract( waveformWidgetRenderer) {

}

GLWaveformRendererSimpleSignal::~GLWaveformRendererSimpleSignal(){
}

void GLWaveformRendererSimpleSignal::init(){
}

void GLWaveformRendererSimpleSignal::setup(const QDomNode &node){
    m_colors.setup(node);

    QColor signalColor = m_colors.getSignalColor();
    signalColor.setAlphaF(0.8);

    QColor bornderColor = m_colors.getSignalColor().lighter(125);
    bornderColor.setAlphaF(0.5);
    m_borderPen.setColor(bornderColor);
    m_borderPen.setWidthF(1.25);

    QLinearGradient gradient(QPointF(0.0,-255.0),QPointF(0.0,255.0));
    gradient.setColorAt(0.0, signalColor);
    gradient.setColorAt(0.25,signalColor.lighter(85));
    gradient.setColorAt(0.5, signalColor.darker(115));
    gradient.setColorAt(0.75,signalColor.lighter(85));
    gradient.setColorAt(1.0, signalColor);
    m_brush = QBrush(gradient);
}

inline void setPoint(QPointF& point, qreal x, qreal y) {
    point.setX(x);
    point.setY(y);
}

void GLWaveformRendererSimpleSignal::draw(QPainter* painter, QPaintEvent* event){

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

    painter->save();

    painter->setRenderHint(QPainter::Antialiasing);

    painter->resetTransform();

    painter->translate(0.0,m_waveformRenderer->getHeight()/2.0);
    painter->scale(1.0,m_waveformRenderer->getGain()*2.0*(double)m_waveformRenderer->getHeight()/255.0);

    const double xOffset = 0.5;

    int pointIndex = 0;

    int firstVisualIndex = m_waveformRenderer->getFirstDisplayedPosition()*dataSize;
    int lastVisualIndex = m_waveformRenderer->getLastDisplayedPosition()*dataSize;

    m_waveformRenderer->regulateVisualSample(firstVisualIndex);
    m_waveformRenderer->regulateVisualSample(lastVisualIndex);

    setPoint(m_polygon[pointIndex], -0.5, 0.0);

    const double offset = (double)firstVisualIndex;
    const double gain = (double)(lastVisualIndex - firstVisualIndex) / (double)m_waveformRenderer->getWidth();

    const double visualSamplePerPixel = m_waveformRenderer->getVisualSamplePerPixel();

    //Rigth channel
    for (double x = 0.0; x < (double)m_waveformRenderer->getWidth(); x++) {
        pointIndex++;

        int visualIndexStart = floor(gain * x + offset - visualSamplePerPixel/2.0);
        visualIndexStart -= visualIndexStart%2;
        int visualIndexStop = ceil(gain * x + offset + visualSamplePerPixel/2.0);

        if (visualIndexStart > 0 && visualIndexStop < dataSize - 1) {
            unsigned char maxValue = 0;

            for( int i = visualIndexStart; i <= visualIndexStop; i += 2) {
                maxValue = math_max(maxValue, (data+i)->filtered.all);
            }

            setPoint(m_polygon[pointIndex], x + xOffset, (float)maxValue);
        }
        else {
            setPoint(m_polygon[pointIndex], x + xOffset, 0.1);
        }
    }

    //pivot point
    pointIndex++;
    setPoint(m_polygon[pointIndex], m_waveformRenderer->getWidth() + xOffset, 0.0);

    //Left channel
    for (double x = (double)m_waveformRenderer->getWidth() - 1.0; x > -1.0; x--) {
        pointIndex++;

        int visualIndexStart = int( gain * x + offset - visualSamplePerPixel/2.0);
        visualIndexStart -= visualIndexStart%2 + 1; //left channel
        int visualIndexStop = int( gain * x + offset + visualSamplePerPixel/2.0);

        if( visualIndexStart > 0 && visualIndexStop < dataSize - 1) {
            unsigned char maxValue = 0;

            for (int i = visualIndexStart; i <= visualIndexStop; i += 2) {
                maxValue = math_max(maxValue, (data+i)->filtered.all);
            }

            setPoint(m_polygon[pointIndex], x + xOffset, -(float)maxValue);
        }
        else {
            setPoint(m_polygon[pointIndex], x + xOffset, -0.1);
        }
    }

    painter->setPen(m_borderPen);
    painter->setBrush(m_brush);

    painter->drawPolygon(&m_polygon[0], pointIndex);

    painter->restore();
}

void GLWaveformRendererSimpleSignal::onResize() {
    m_polygon.resize(2*m_waveformRenderer->getWidth()+2);
}

