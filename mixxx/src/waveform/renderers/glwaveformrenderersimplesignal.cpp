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

void GLWaveformRendererSimpleSignal::draw(QPainter* painter, QPaintEvent* event){

    const TrackInfoObject* trackInfo = m_waveformRenderer->getTrackInfo().data();

    if( !trackInfo)
        return;

    painter->save();

    painter->setRenderHint(QPainter::Antialiasing);

    painter->resetTransform();

    painter->translate(0.0,m_waveformRenderer->getHeight()/2.0);
    painter->scale(1.0,m_waveformRenderer->getGain()*2.0*(double)m_waveformRenderer->getHeight()/255.0);

    const Waveform* waveform = m_waveformRenderer->getTrackInfo()->getWaveform();

    int pointIndex = 0;

    int firstVisualIndex = m_waveformRenderer->getFirstDisplayedPosition()*waveform->size();
    int lastVisualIndex = m_waveformRenderer->getLastDisplayedPosition()*waveform->size();

    m_waveformRenderer->regulateVisualSample(firstVisualIndex);
    m_waveformRenderer->regulateVisualSample(lastVisualIndex);

    m_polygon[pointIndex] = QPointF(-0.5,0.0);

    const double offset = (double)firstVisualIndex;
    const double gain = (double)(lastVisualIndex - firstVisualIndex) / (double)m_waveformRenderer->getWidth();

    const double visualSamplePerPixel = m_waveformRenderer->getVisualSamplePerPixel();

    //Rigth channel
    for( double x = 0.0; x < (double)m_waveformRenderer->getWidth(); x++)
    {
        pointIndex++;

        int visualIndexStart = floor( gain * x + offset - visualSamplePerPixel/2.0);
        visualIndexStart -= visualIndexStart%2;
        int visualIndexStop = ceil( gain * x + offset + visualSamplePerPixel/2.0);

        if( visualIndexStart > 0 && visualIndexStop < waveform->size() - 1) {
            unsigned char maxValue = 0;

            for( int i = visualIndexStart; i <= visualIndexStop; i+=2)
                maxValue = math_max( maxValue, waveform->getAll(i));

            m_polygon[pointIndex] = QPointF(x+0.5,(float)maxValue);
        }
        else {
            m_polygon[pointIndex] = QPointF(x+0.5,0.0);
        }
    }

    //pivot point
    pointIndex++;
    m_polygon[pointIndex] = QPointF(m_waveformRenderer->getWidth()+0.5,0.0);

    //Left channel
    for( double x = (double)m_waveformRenderer->getWidth() - 1.0; x > -1.0; x--)
    {
        pointIndex++;

        int visualIndexStart = int( gain * x + offset - visualSamplePerPixel/2.0);
        visualIndexStart -= visualIndexStart%2 + 1; //left channel
        int visualIndexStop = int( gain * x + offset + visualSamplePerPixel/2.0);

        if( visualIndexStart > 0 && visualIndexStop < waveform->size() - 1) {
            unsigned char maxValue = 0;

            for( int i = visualIndexStart; i <= visualIndexStop; i+=2)
                maxValue = math_max( maxValue, waveform->getAll(i));

            m_polygon[pointIndex] = QPointF(x+0.5,-(float)maxValue);
        }
        else {
            m_polygon[pointIndex] = QPointF(x+0.5,0.0);
        }
    }

    painter->setPen(m_borderPen);
    painter->setBrush(m_brush);

    painter->drawPolygon(&m_polygon[0],pointIndex);

    painter->restore();
}

void GLWaveformRendererSimpleSignal::onResize() {
    m_polygon.resize(2*m_waveformRenderer->getWidth()+2);
}

