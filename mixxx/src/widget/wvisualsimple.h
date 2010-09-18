/***************************************************************************
                          wvisualsimple.h  -  description
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

#ifndef WVISUALSIMPLE_H
#define WVISUALSIMPLE_H

#include "wwidget.h"
#include <qpoint.h>
#include <qrect.h>
//Added by qt3to4:
#include <QPaintEvent>
#include <QMouseEvent>
#include <QDropEvent>
#include <QDragEnterEvent>

/**
  *@author Tue & Ken Haste Andersen
  */

class WaveformRenderer;

class WVisualSimple : public WWidget
{
    Q_OBJECT
  public:
    WVisualSimple(const char* group, QWidget *parent, WaveformRenderer* pWaveformRenderer);
    virtual ~WVisualSimple();

    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void setup(QDomNode node);
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *);

  signals:
    void trackDropped(QString filename, QString group);

  public slots:
    void setValue(double) {};
    void refresh();


  protected:
    QString m_group;
    int m_iStartPosX, m_iValue;
    QPoint m_qMarkerPos1, m_qMarkerPos2, m_qMousePos;

    /** Colors */
    QColor colorSignal, colorMarker;

    WaveformRenderer *m_pWaveformRenderer;
};

#endif
