#include "waveformrendererendoftrack.h"

#include <QDebug>

#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>

#include "waveformwidgetrenderer.h"

#include "waveformrenderer.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

WaveformRendererEndOfTrack::WaveformRendererEndOfTrack( WaveformWidgetRenderer* waveformWidgetRenderer) :
    WaveformRendererAbstract(waveformWidgetRenderer),
    m_color(200,25,20),
    m_blinkingPeriod(1000), //1sec
    m_remainingTimeTrigger(30.0) //30sec
{
}

void WaveformRendererEndOfTrack::init()
{
    m_timer.restart();
}

void WaveformRendererEndOfTrack::setup( const QDomNode& /*node*/)
{
    //TODO: add EnfOfTrack color in skins and why not blinking period too
}

void WaveformRendererEndOfTrack::draw( QPainter* painter, QPaintEvent* /*event*/)
{
    if( isDirty())
        generatePixmap();

    if( !m_waveformWidget->getTrackInfo())
        return;

    int remainingSamples = (1.0-m_waveformWidget->getPlayPos())*0.5*m_waveformWidget->getTrackSamples();
    double remainingTime = (double)remainingSamples/m_waveformWidget->getTrackInfo()->getSampleRate();

    if( remainingTime > m_remainingTimeTrigger)
        return;

    //TODO vRince : add some logic about direction, play/stop, loop ?

    //NOTE vRince : why don't we use a nice QAnimation ?
    int elapsed = m_timer.elapsed();
    elapsed %= m_blinkingPeriod;
    int index = s_maxAlpha*(double)abs(elapsed-m_blinkingPeriod/2)/(double)(m_blinkingPeriod/2);
    index = index < s_maxAlpha - 1 ? index : s_maxAlpha - 1;
    index = index > 0 ? index : 0;

    painter->drawPixmap( QPoint(m_waveformWidget->getWidth()-m_pixmaps[index].width(),0), m_pixmaps[index]);
}

void WaveformRendererEndOfTrack::generatePixmap()
{
    //qDebug() << "WaveformRendererEndOfTrack::generatePixmap";

    for( int i = 0; i < s_maxAlpha; ++i)
    {
        m_pixmaps[i] = QPixmap( m_waveformWidget->getWidth()/4, m_waveformWidget->getHeight());
        m_pixmaps[i].fill(QColor(0,0,0,0));

        QColor startColor = m_color;
        startColor.setAlpha(0);
        QColor endcolor = m_color;

        QRadialGradient gradBackground(QPointF(1.5*m_pixmaps[i].width(),m_pixmaps[i].height()/2),1.5*m_pixmaps[i].width());
        endcolor.setAlpha(i);
        gradBackground.setColorAt(1.0,startColor);
        gradBackground.setColorAt(0.0,endcolor);

        QLinearGradient linearGradBorder(QPointF(0,0),QPointF(m_pixmaps[i].width(),0));
        linearGradBorder.setColorAt(0.0,startColor);
        linearGradBorder.setColorAt(1.0,endcolor);

        QBrush brush(gradBackground);
        QPen pen(linearGradBorder,2.0);

        QRectF rectangle(2.5,2.5,(float)m_pixmaps[i].width()-5.0,(float)m_pixmaps[i].height()-5.0);

        QPainter painter;
        painter.begin(&m_pixmaps[i]);
        painter.setRenderHint(QPainter::HighQualityAntialiasing,true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform,true);
        painter.setBrush(brush);
        painter.setPen(pen);
        painter.drawRoundedRect(rectangle,9.5,9.5);

        //debug
        //m_pixmaps[i].save("pixmaps/pixmap_" + QString::number(i) + ".png");
    }

    setDirty(false);
}
