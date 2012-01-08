#include "glwaveformrendererfilteredsignal.h"

#include "waveformwidgetrenderer.h"
#include "waveform/waveform.h"

#include "widget/wskincolor.h"
#include "trackinfoobject.h"
#include "widget/wwidget.h"

#include "controlobject.h"
#include "defs.h"

#include <QLinearGradient>
#include <QLineF>

GLWaveformRendererFilteredSignal::GLWaveformRendererFilteredSignal( WaveformWidgetRenderer* waveformWidgetRenderer) :
    WaveformRendererAbstract( waveformWidgetRenderer) {
    m_lowFilterControlObject = 0;
    m_midFilterControlObject = 0;
    m_highFilterControlObject = 0;
    m_lowKillControlObject = 0;
    m_midKillControlObject = 0;
    m_highKillControlObject = 0;
}

void GLWaveformRendererFilteredSignal::init() {
    //create controls
    m_lowFilterControlObject = ControlObject::getControl( ConfigKey(m_waveformRenderer->getGroup(),"filterLow"));
    m_midFilterControlObject = ControlObject::getControl( ConfigKey(m_waveformRenderer->getGroup(),"filterMid"));
    m_highFilterControlObject = ControlObject::getControl( ConfigKey(m_waveformRenderer->getGroup(),"filterHigh"));
    m_lowKillControlObject = ControlObject::getControl( ConfigKey(m_waveformRenderer->getGroup(),"filterLowKill"));
    m_midKillControlObject = ControlObject::getControl( ConfigKey(m_waveformRenderer->getGroup(),"filterMidKill"));
    m_highKillControlObject = ControlObject::getControl( ConfigKey(m_waveformRenderer->getGroup(),"filterHighKill"));
}

void GLWaveformRendererFilteredSignal::setup( const QDomNode& node) {

    m_signalColor.setNamedColor(WWidget::selectNodeQString(node, "SignalColor"));
    m_signalColor = WSkinColor::getCorrectColor(m_signalColor);

    //TODO vRince: fetch color from skin
    int h,s,l;
    m_signalColor.getHsl(&h,&s,&l);

    QLinearGradient gradientLow(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientLow.setColorAt(0.0, QColor::fromHsl(h,s,60,120));
    gradientLow.setColorAt(0.25,QColor::fromHsl(h,s,50,120));
    gradientLow.setColorAt(0.5, QColor::fromHsl(h,s,30,120));
    gradientLow.setColorAt(0.75,QColor::fromHsl(h,s,50,120));
    gradientLow.setColorAt(1.0, QColor::fromHsl(h,s,60,120));
    m_lowBrush = QBrush(gradientLow);

    QLinearGradient gradientMid(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientMid.setColorAt(0.0, QColor::fromHsl(h-5,s,110,120));
    gradientMid.setColorAt(0.25,QColor::fromHsl(h-5,s,100,120));
    gradientMid.setColorAt(0.5, QColor::fromHsl(h-5,s, 80,120));
    gradientMid.setColorAt(0.75,QColor::fromHsl(h-5,s,100,120));
    gradientMid.setColorAt(1.0, QColor::fromHsl(h-5,s,100,120));
    m_midBrush = QBrush(gradientMid);

    QLinearGradient gradientHigh(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientHigh.setColorAt(0.0, QColor::fromHsl(h+5,s,210,120));
    gradientHigh.setColorAt(0.25,QColor::fromHsl(h+5,s,200,120));
    gradientHigh.setColorAt(0.5, QColor::fromHsl(h+5,s,180,120));
    gradientHigh.setColorAt(0.75,QColor::fromHsl(h+5,s,200,120));
    gradientHigh.setColorAt(1.0, QColor::fromHsl(h+5,s,210,120));
    m_highBrush = QBrush(gradientHigh);

    QLinearGradient gradientKilledLow(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientKilledLow.setColorAt(0.0, QColor::fromHsl(h,s,30,80));
    gradientKilledLow.setColorAt(0.5, QColor(200,200,200,10));
    gradientKilledLow.setColorAt(1.0, QColor::fromHsl(h,s,30,70));
    m_lowKilledBrush = QBrush(gradientKilledLow);

    QLinearGradient gradientKilledMid(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientKilledMid.setColorAt(0.0, QColor::fromHsl(h-5,s, 80,80));
    gradientKilledMid.setColorAt(0.5, QColor(200,200,200,10));
    gradientKilledMid.setColorAt(1.0, QColor::fromHsl(h-5,s, 80,70));
    m_midKilledBrush = QBrush(gradientKilledMid);

    QLinearGradient gradientKilledHigh(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientKilledHigh.setColorAt(0.0, QColor::fromHsl(h+5,s,180,80));
    gradientKilledHigh.setColorAt(0.5, QColor(200,200,200,10));
    gradientKilledHigh.setColorAt(1.0, QColor::fromHsl(h+5,s,180,70));
    m_highKilledBrush = QBrush(gradientKilledHigh);
}

void GLWaveformRendererFilteredSignal::onResize() {
    m_polygon[0].resize(2*m_waveformRenderer->getWidth()+2);
    m_polygon[1].resize(2*m_waveformRenderer->getWidth()+2);
    m_polygon[2].resize(2*m_waveformRenderer->getWidth()+2);
}

int GLWaveformRendererFilteredSignal::buildPolygon() {

    const Waveform* waveform = m_waveformRenderer->getTrackInfo()->getWaveform();

    int pointIndex = 0;

    const double firstVisualIndex = m_waveformRenderer->getFirstDisplayedPosition()*waveform->size();
    const double lastVisualIndex = m_waveformRenderer->getLastDisplayedPosition()*waveform->size();

    m_polygon[0][pointIndex] = QPointF(0.0,0.0);
    m_polygon[1][pointIndex] = QPointF(0.0,0.0);
    m_polygon[2][pointIndex] = QPointF(0.0,0.0);

    const double offset = firstVisualIndex;
    const double gain = (lastVisualIndex - firstVisualIndex) / (double)m_waveformRenderer->getWidth();

    const double visualSamplePerPixel = m_waveformRenderer->getVisualSamplePerPixel();
    const int visaulSamplePerDemiPixel = int(visualSamplePerPixel / 2.0); //this brings viaul stability (less flickering)

    float lowGain(1.0), midGain(1.0), highGain(1.0);
    if( m_lowFilterControlObject && m_midFilterControlObject && m_highFilterControlObject){
        lowGain = m_lowFilterControlObject->get();
        midGain = m_midFilterControlObject->get();
        highGain = m_highFilterControlObject->get();
    }

    //Rigth channel
    for( int x = 0; x < m_waveformRenderer->getWidth(); x++)
    {
        pointIndex++;

        int visualIndexStart = int( gain * (double)(x) + offset - visualSamplePerPixel/2.0);
        visualIndexStart -= visualIndexStart%2; //rigth channel
        int visualIndexStop = int( gain * (double)(x) + offset + visualSamplePerPixel/2.0);

        if( visualIndexStart > 0 && visualIndexStop + visaulSamplePerDemiPixel < waveform->size() -1) {
            unsigned char maxLow = 0;
            unsigned char maxBand = 0;
            unsigned char maxHigh = 0;

            for( int i = visualIndexStart; i <= visualIndexStop + visaulSamplePerDemiPixel; i+=2) {
                maxLow = math_max( maxLow, waveform->getLow(i));
                maxBand = math_max( maxBand, waveform->getMid(i));
                maxHigh = math_max( maxHigh, waveform->getHigh(i));
            }

            m_polygon[0][pointIndex] = QPointF(x,(float)maxLow*lowGain);
            m_polygon[1][pointIndex] = QPointF(x,(float)maxBand*midGain);
            m_polygon[2][pointIndex] = QPointF(x,(float)maxHigh*highGain);
        }
        else {
            m_polygon[0][pointIndex] = QPointF(x,0.0);
            m_polygon[1][pointIndex] = QPointF(x,0.0);
            m_polygon[2][pointIndex] = QPointF(x,0.0);
        }
    }

    //pivot point
    pointIndex++;
    m_polygon[0][pointIndex] = QPointF(m_waveformRenderer->getWidth(),0.0);
    m_polygon[1][pointIndex] = QPointF(m_waveformRenderer->getWidth(),0.0);
    m_polygon[2][pointIndex] = QPointF(m_waveformRenderer->getWidth(),0.0);

    //Left channel
    for( int x = m_waveformRenderer->getWidth() - 1; x > -1; x--)
    {
        pointIndex++;

        int visualIndexStart = int( gain * (double)(x) + offset - visualSamplePerPixel/2.0);
        visualIndexStart -= visualIndexStart%2 + 1; //left channel
        int visualIndexStop = int( gain * (double)(x) + offset + visualSamplePerPixel/2.0);

        if( visualIndexStart > 0 && visualIndexStop + visaulSamplePerDemiPixel < waveform->size() -1) {
            unsigned char maxLow = 0;
            unsigned char maxBand = 0;
            unsigned char maxHigh = 0;

            for( int i = visualIndexStart; i <= visualIndexStop + visaulSamplePerDemiPixel; i+=2) {
                maxLow = math_max( maxLow, waveform->getLow(i));
                maxBand = math_max( maxBand, waveform->getMid(i));
                maxHigh = math_max( maxHigh, waveform->getHigh(i));
            }
            m_polygon[0][pointIndex] = QPointF(x,-(float)maxLow*lowGain);
            m_polygon[1][pointIndex] = QPointF(x,-(float)maxBand*midGain);
            m_polygon[2][pointIndex] = QPointF(x,-(float)maxHigh*highGain);
        }
        else {
            m_polygon[0][pointIndex] = QPointF(x,0.0);
            m_polygon[1][pointIndex] = QPointF(x,0.0);
            m_polygon[2][pointIndex] = QPointF(x,0.0);
        }
    }

    return pointIndex;
}

void GLWaveformRendererFilteredSignal::draw(QPainter* painter, QPaintEvent* /*event*/) {

    const TrackInfoObject* trackInfo = m_waveformRenderer->getTrackInfo().data();

    if( !trackInfo)
        return;

    painter->save();

    painter->setRenderHint( QPainter::Antialiasing);
    painter->resetTransform();

    painter->translate(0.0,m_waveformRenderer->getHeight()/2.0);
    painter->scale(1.0,m_waveformRenderer->getGain()*2.0*(double)m_waveformRenderer->getHeight()/255.0);

    int numberOfPoints = buildPolygon();

    if( m_lowKillControlObject && m_lowKillControlObject->get() > 0.1) {
        painter->setPen( QPen( m_lowKilledBrush, 0.0));
        painter->setBrush(QColor(150,150,150,20));
    }
    else {
        painter->setPen( QPen( m_lowBrush, 0.0));
        painter->setBrush( m_lowBrush);
    }
    painter->drawPolygon(&m_polygon[0][0],numberOfPoints);

    if( m_midKillControlObject && m_midKillControlObject->get() > 0.1) {
        painter->setPen( QPen( m_midKilledBrush, 0.0));
        painter->setBrush(QColor(150,150,150,20));
    }
    else {
        painter->setPen( QPen( m_midBrush, 0.0));
        painter->setBrush( m_midBrush);
    }
    painter->drawPolygon(&m_polygon[1][0],numberOfPoints);

    if( m_highKillControlObject && m_highKillControlObject->get() > 0.1) {
        painter->setPen( QPen( m_highKilledBrush, 0.0));
        painter->setBrush(QColor(150,150,150,20));
    }
    else {
        painter->setPen( QPen( m_highBrush, 0.0));
        painter->setBrush( m_highBrush);
    }
    painter->drawPolygon(&m_polygon[2][0],numberOfPoints);

    painter->restore();
}
