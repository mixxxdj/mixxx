// waveformrendermarkrange.cpp
// Created 11/14/2009 by RJ Ryan (rryan@mit.edu)

#include <QDebug>
#include <QColor>
#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>
#include <QObject>
#include <QVector>

#include "waveformrendermarkrange.h"

#include "configobject.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"
#include "trackinfoobject.h"

#include "waveformwidgetrenderer.h"

MarkRange::MarkRange()
{
    m_markStartPoint = 0;
    m_markEndPoint = 0;
    m_markEnabled = 0;
}

void MarkRange::generatePixmap( int weidth, int height)
{
    m_activePixmap = QPixmap( weidth, height);
    m_disabledPixmap = QPixmap( weidth, height);

    //fill needed cause they remain transparent
    m_activePixmap.fill(QColor(0,0,0,0));
    m_disabledPixmap.fill(QColor(0,0,0,0));

    QColor activeBorderColor = m_activeColor.lighter(50);
    activeBorderColor.setAlphaF(0.3);
    QColor activeCenterColor = m_activeColor;
    activeCenterColor.setAlphaF(0.05);

    QLinearGradient linearGrad(QPointF(0,0), QPointF(0,height));
    linearGrad.setColorAt(0.0, activeCenterColor);
    linearGrad.setColorAt(0.01, activeBorderColor);
    linearGrad.setColorAt(0.4, activeCenterColor);
    linearGrad.setColorAt(0.6, activeCenterColor);
    linearGrad.setColorAt(0.99, activeBorderColor);
    linearGrad.setColorAt(1.0, activeCenterColor);

    QBrush brush(linearGrad);

    QPainter painter;
    painter.begin(&m_activePixmap);
    painter.fillRect(m_activePixmap.rect(), brush);
    painter.end();

    QColor disabledBorderColor = m_disabledColor;
    disabledBorderColor.setAlphaF(0.2);
    QColor disabledCenterColor = m_disabledColor.darker(100);
    disabledCenterColor.setAlphaF(0.05);

    linearGrad = QLinearGradient(QPointF(0,0), QPointF(0,height));
    linearGrad.setColorAt(0.0, disabledBorderColor);
    linearGrad.setColorAt(0.2, disabledCenterColor);
    linearGrad.setColorAt(0.8, disabledCenterColor);
    linearGrad.setColorAt(1.0, disabledBorderColor);

    brush = QBrush(linearGrad);

    painter.begin(&m_disabledPixmap);
    painter.fillRect(m_disabledPixmap.rect(), brush);
    painter.end();
}

/////////////////////////////////

WaveformRenderMarkRange::WaveformRenderMarkRange( WaveformWidgetRenderer* waveformWidgetRenderer) :
    WaveformRendererAbstract(waveformWidgetRenderer)
{
}

void WaveformRenderMarkRange::init()
{
}

void WaveformRenderMarkRange::setup(const QDomNode &node)
{
    markRanges_.clear();
    markRanges_.reserve(32);

    QDomNode child = node.firstChild();
    while (!child.isNull())
    {
        if (child.nodeName() == "MarkRange")
        {
            markRanges_.push_back( MarkRange());
            setupMarkRange(child,markRanges_.back());
        }
        child = child.nextSibling();
    }
}

void WaveformRenderMarkRange::draw(QPainter *painter, QPaintEvent * /*event*/)
{
    painter->save();

    painter->setWorldMatrixEnabled(false);

    if( isDirty())
        generatePixmaps();

    //make the painter in the track position 'world'
    double w = m_waveformWidget->getWidth();
    double s = m_waveformWidget->getFirstDisplayedPosition();
    double e = m_waveformWidget->getLastDisplayedPosition();

    float a = w/(e-s);
    float b = -s;

    for( int i = 0; i < markRanges_.size(); i++)
    {
        MarkRange& markRange = markRanges_[i];

        if( !markRange.isValid())
            continue;

        int startSample = markRange.m_markStartPoint->get();
        int endSample = markRange.m_markEndPoint->get();
        if( startSample < 0 || endSample < 0)
            continue;

        m_waveformWidget->regulateAudioSample(startSample);
        double startPosition = (double)startSample / (double)m_waveformWidget->getTrackSamples();

        m_waveformWidget->regulateAudioSample(endSample);
        double endPosition = (double)endSample / (double)m_waveformWidget->getTrackSamples();

        //range not in the current display
        if( startPosition > m_waveformWidget->getLastDisplayedPosition() ||
                endPosition < m_waveformWidget->getFirstDisplayedPosition())
            continue;

        startPosition = a*(startPosition+b);
        endPosition = a*(endPosition+b);

        QPixmap* selectedPixmap = 0;

        if( markRange.m_markEnabled && markRange.m_markEnabled->get() < 0.5)
            selectedPixmap = &markRange.m_disabledPixmap;
        else
            selectedPixmap = &markRange.m_activePixmap;

        //draw the correcponding portion of the selected pixmap
        //this shouldn't involve *any* scaling it should be fast even in software more
        QRect rect(startPosition,0,endPosition-startPosition,m_waveformWidget->getHeight());
        painter->drawPixmap( rect, *selectedPixmap, rect);
    }

    painter->restore();
}

void WaveformRenderMarkRange::setupMarkRange(const QDomNode &node, MarkRange &markRange)
{
    markRange.m_activeColor = WWidget::selectNodeQString(node, "Color");
    if( markRange.m_activeColor == "") {
        //vRince kinf of legacy fallback ...
        // As a fallback, grab the mark color from the parent's MarkerColor
        markRange.m_activeColor = WWidget::selectNodeQString(node.parentNode(), "MarkerColor");
        qDebug() << "Didn't get mark Color, using parent's MarkerColor:" << markRange.m_activeColor;
    }

    markRange.m_disabledColor = WWidget::selectNodeQString(node, "DisabledColor");
    if( markRange.m_disabledColor == "") {
        //vRince kinf of legacy fallback ...
        // Read the text color, otherwise use the parent's SignalColor.
        markRange.m_disabledColor = WWidget::selectNodeQString(node.parentNode(), "SignalColor");
        qDebug() << "Didn't get mark TextColor, using parent's SignalColor:" << markRange.m_disabledColor;
    }

    markRange.m_markStartPoint = ControlObject::getControl(
                ConfigKey(m_waveformWidget->getGroup(),
                          WWidget::selectNodeQString(node, "StartControl")));
    markRange.m_markEndPoint = ControlObject::getControl(
                ConfigKey(m_waveformWidget->getGroup(),
                          WWidget::selectNodeQString(node, "EndControl")));
    markRange.m_markEnabled = ControlObject::getControl(
                ConfigKey(m_waveformWidget->getGroup(),
                          WWidget::selectNodeQString(node, "EnabledControl")));
}

void WaveformRenderMarkRange::generatePixmaps()
{
    for( int i = 0; i < markRanges_.size(); i++)
        markRanges_[i].generatePixmap( m_waveformWidget->getWidth(), m_waveformWidget->getHeight());
    setDirty(false);
}

