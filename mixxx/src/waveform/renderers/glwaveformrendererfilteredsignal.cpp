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

    m_colors.setup(node);

    QColor low = m_colors.getLowColor();
    QColor mid = m_colors.getMidColor();
    QColor high = m_colors.getHighColor();

    QColor lowCenter = low;
    QColor midCenter = mid;
    QColor highCenter = high;

    low.setAlphaF(0.6);
    mid.setAlphaF(0.6);
    high.setAlphaF(0.6);

    lowCenter.setAlphaF(0.2);
    midCenter.setAlphaF(0.3);
    highCenter.setAlphaF(0.3);

    QLinearGradient gradientLow(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientLow.setColorAt(0.0, low);
    gradientLow.setColorAt(0.25,low.lighter(85));
    gradientLow.setColorAt(0.5, lowCenter.darker(115));
    gradientLow.setColorAt(0.75,low.lighter(85));
    gradientLow.setColorAt(1.0, low);
    m_lowBrush = QBrush(gradientLow);

    QLinearGradient gradientMid(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientMid.setColorAt(0.0, mid);
    gradientMid.setColorAt(0.35,mid.lighter(85));
    gradientMid.setColorAt(0.5, midCenter.darker(115));
    gradientMid.setColorAt(0.65,mid.lighter(85));
    gradientMid.setColorAt(1.0, mid);
    m_midBrush = QBrush(gradientMid);

    QLinearGradient gradientHigh(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientHigh.setColorAt(0.0, high);
    gradientHigh.setColorAt(0.45,high.lighter(85));
    gradientHigh.setColorAt(0.5, highCenter.darker(115));
    gradientHigh.setColorAt(0.55,high.lighter(85));
    gradientHigh.setColorAt(1.0, high);
    m_highBrush = QBrush(gradientHigh);

    low.setAlphaF(0.3);
    mid.setAlphaF(0.3);
    high.setAlphaF(0.3);

    QLinearGradient gradientKilledLow(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientKilledLow.setColorAt(0.0,low.darker(80));
    gradientKilledLow.setColorAt(0.5,lowCenter.darker(150));
    gradientKilledLow.setColorAt(1.0,low.darker(80));
    m_lowKilledBrush = QBrush(gradientKilledLow);

    QLinearGradient gradientKilledMid(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientKilledMid.setColorAt(0.0,mid.darker(80));
    gradientKilledMid.setColorAt(0.5,midCenter.darker(150));
    gradientKilledMid.setColorAt(1.0,mid.darker(80));
    m_midKilledBrush = QBrush(gradientKilledMid);

    QLinearGradient gradientKilledHigh(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientKilledHigh.setColorAt(0.0,high.darker(80));
    gradientKilledHigh.setColorAt(0.5,highCenter.darker(150));
    gradientKilledHigh.setColorAt(1.0,high.darker(80));
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
    const int visualSamplePerDemiPixel = ceil(visualSamplePerPixel / 2.0); //this brings viul stability (less flickering)

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

        if( visualIndexStart > 0 && visualIndexStop + visualSamplePerDemiPixel < waveform->size() -1) {
            unsigned char maxLow = 0;
            unsigned char maxBand = 0;
            unsigned char maxHigh = 0;

            for( int i = visualIndexStart; i <= visualIndexStop + visualSamplePerDemiPixel; i+=2) {
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

        if( visualIndexStart > 0 && visualIndexStop + visualSamplePerDemiPixel < waveform->size() -1) {
            unsigned char maxLow = 0;
            unsigned char maxBand = 0;
            unsigned char maxHigh = 0;

            for( int i = visualIndexStart; i <= visualIndexStop + visualSamplePerDemiPixel; i+=2) {
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
