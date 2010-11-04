/***************************************************************************
                          wvisualsimple.cpp  -  description
                             -------------------
    begin                : Thu Oct 9 2003
    copyright            : (C) 2002 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "wvisualsimple.h"
#include "wskincolor.h"
#include <qpainter.h>
#include <qpixmap.h>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QUrl>

#include "waveform/waveformrenderer.h"

WVisualSimple::WVisualSimple(const char* group, QWidget * parent, WaveformRenderer* pWaveformRenderer)
        : WWidget(parent),
          m_group(group),
          m_iValue(64),
          m_pWaveformRenderer(pWaveformRenderer)

{
    setAcceptDrops(true);
}

WVisualSimple::~WVisualSimple()
{
}

void WVisualSimple::dragEnterEvent(QDragEnterEvent * event)
{
  if (event->mimeData()->hasUrls())
      event->acceptProposedAction();
}

void WVisualSimple::dropEvent(QDropEvent * event)
{
  if (event->mimeData()->hasUrls()) {
    QList<QUrl> urls(event->mimeData()->urls());
    QUrl url = urls.first();
    QString name = url.toLocalFile();

    event->accept();
    emit(trackDropped(name, m_group));
  } else
    event->ignore();
}

void WVisualSimple::setup(QDomNode node)
{
    int w = width(), h = height();

    m_pWaveformRenderer->resize(w,h);
    m_pWaveformRenderer->setup(node);

    // Set constants for line drawing
    m_qMarkerPos1.setX(w/2);
    m_qMarkerPos1.setY(0);
    m_qMarkerPos2.setX(w/2);
    m_qMarkerPos2.setY(h);
    m_qMousePos.setX(w/2);
    m_qMousePos.setY(h/2);

    // Background color
    QColor c(255,255,255);
    if (!selectNode(node, "BgColor").isNull())
    {
        c.setNamedColor(selectNodeQString(node, "BgColor"));
    }

    //the simple view seems to look fine even if we never set a background colour at all
    //but since the code used to do it, we'll continue to do it --kousu 2009/03
    QPalette palette = this->palette();
    //setBackgroundColor(WSkinColor::getCorrectColor(c));
    palette.setColor(this->backgroundRole(), WSkinColor::getCorrectColor(c));
    setPalette(palette);

    colorSignal.setNamedColor(selectNodeQString(node, "SignalColor"));
    colorSignal = WSkinColor::getCorrectColor(colorSignal);
    colorMarker.setNamedColor(selectNodeQString(node, "MarkerColor"));
    colorMarker = WSkinColor::getCorrectColor(colorMarker);
}

void WVisualSimple::refresh() {
    // Do nothing since its the simple waveform.
}

void WVisualSimple::mouseMoveEvent(QMouseEvent * e)
{
    // Only process mouse move if it was initiated by a left click
    if (m_iStartPosX!=-1)
    {
        double v = 64.+(double)(e->x()-m_iStartPosX)/10.;
        if (v<0.)
            v = 0.;
        else if (v>127.)
            v = 127.;
        emit(valueChangedLeftDown(v));
        m_iValue = (int)v;
    }
    update();
}

void WVisualSimple::mousePressEvent(QMouseEvent * e)
{
    m_iStartPosX = -1;
    if (e->button()==Qt::LeftButton)
    {
        // Store current x position of mouse pointer
        m_iStartPosX = e->x();
        emit(valueChangedLeftDown(64.));
    }
}

void WVisualSimple::mouseReleaseEvent(QMouseEvent *)
{
    m_iValue = 64;
    emit(valueChangedLeftDown((double)m_iValue));
    update();
}

void WVisualSimple::paintEvent(QPaintEvent *)
{
    QPainter paint(this);

    // Draw vertical red bar in center
    paint.setPen(colorMarker);
    paint.drawLine(m_qMarkerPos1, m_qMarkerPos2);

    // Draw lines indicating position
    if (m_iValue!=64)
    {
        paint.setPen(colorSignal);
        QPoint p1 = m_qMousePos+QPoint((m_iValue-64)*2,0);
        QPoint p2 = m_qMousePos+QPoint((m_iValue-64)*2,0);
        paint.drawLine(m_qMarkerPos1, p1);
        paint.drawLine(m_qMarkerPos2, p2);
    }
}



