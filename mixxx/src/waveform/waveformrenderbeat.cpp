#include "waveformrenderbeat.h"

#include "waveformwidgetrenderer.h"
#include "widget/wskincolor.h"
#include "trackinfoobject.h"
#include "widget/wwidget.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"

#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>

WaveformRenderBeat::WaveformRenderBeat( WaveformWidgetRenderer* waveformWidgetRenderer) :
    WaveformRendererAbstract(waveformWidgetRenderer),
    m_beatActive(0){
}

WaveformRenderBeat::~WaveformRenderBeat(){
    if( m_beatActive)
        delete m_beatActive;
}

void WaveformRenderBeat::init(){
    m_beatActive = new ControlObjectThreadMain(
                ControlObject::getControl( ConfigKey(m_waveformWidget->getGroup(),"beat_active")));
}

void WaveformRenderBeat::setup( const QDomNode& node){
    m_beatColor.setNamedColor(WWidget::selectNodeQString(node, "BeatColor"));
    m_beatColor = WSkinColor::getCorrectColor(m_beatColor);

    m_highBeatColor = Qt::black;
    QString highlight = WWidget::selectNodeQString(node, "BeatHighlightColor");
    if( highlight != "") {
        m_highBeatColor.setNamedColor(highlight);
    }
    m_highBeatColor = WSkinColor::getCorrectColor(m_highBeatColor);

    if( m_beatColor.alphaF() > 0.99)
        m_beatColor.setAlphaF(0.6);

    if( m_highBeatColor.alphaF() > 0.99)
        m_highBeatColor.setAlphaF(0.9);
}

void WaveformRenderBeat::draw( QPainter* painter, QPaintEvent* /*event*/){

    TrackPointer trackInfo = m_waveformWidget->getTrackInfo();

    if(!trackInfo)
        return;

    BeatsPointer trackBeats = trackInfo->getBeats();
    if( !trackBeats)
        return;

    m_beatsCache.clear();
    trackBeats->findBeats( m_waveformWidget->getFirstDisplayedPosition() * m_waveformWidget->getTrackSamples(),
                           m_waveformWidget->getLastDisplayedPosition() * m_waveformWidget->getTrackSamples(),
                           &m_beatsCache);

    //if no beat do not waste time saving/restoring painter
    if( m_beatsCache.isEmpty())
        return;

    painter->save();

    for( Const_BeatIterator it = m_beatsCache.begin(); it != m_beatsCache.end(); it++)
    {
        int beatPosition = *it;
        m_waveformWidget->regulateVisualSample(beatPosition);
        double xBeatPoint = m_waveformWidget->transformSampleIndexInRendererWorld(beatPosition);

        //NOTE (vRince) : RJ should we keep this ?
        if( m_beatActive && m_beatActive->get() > 0.0 && abs(xBeatPoint - m_waveformWidget->getWidth()/2) < 20)
            painter->setPen(QColor(m_highBeatColor));
        else
            painter->setPen(QColor(m_beatColor));

        painter->drawLine(xBeatPoint,0.0,xBeatPoint,m_waveformWidget->getHeight());
    }

    painter->restore();
}
