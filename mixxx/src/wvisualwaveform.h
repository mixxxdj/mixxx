/***************************************************************************
                          wvisualwaveform.h  - Waveform visualization
                          using OpenGL. This object cannot make use of
                          multiple inheritance from both WWidget and
                          QGLWidget, since both in turn inherit from
                          QObject.
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

#ifndef WVISUALWAVEFORM_H
#define WVISUALWAVEFORM_H

#include <qgl.h>
#include <q3ptrlist.h>
#include <qevent.h>
#include <qdatetime.h>
#include <qdom.h>
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QTimerEvent>
#include "wwidget.h"
#include "visual/visualcontroller.h"
#include "visual/picking.h"
#include "visual/visualbackplane.h"
#include "visual/visualchannel.h"

class ControlPotmeter;

#include "defs.h"

/**
  *@author Tue & Ken Haste Andersen
  */

class WVisualWaveform : public QGLWidget
{
    Q_OBJECT
public:
    WVisualWaveform(QWidget *pParent=0, const char *pName=0, const QGLWidget *pShareWidget = 0);
    ~WVisualWaveform();
    /** Returns true if direct rendering is enabled */
    bool directRendering();
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void setup(QDomNode node);
    bool eventFilter(QObject *o, QEvent *e);
    /** Add a VisualChannel */
    VisualChannel *add(const char *group);
	void resetColors();

public slots:
    void setValue(double) {};
    void slotNewTrack();
signals:
    void valueChangedLeftDown(double);
    void valueChangedRightDown(double);
    void trackDropped(QString filename);

protected:
    void initializeGL();
    void resizeGL(int, int);
    void paintGL();
    void timerEvent(QTimerEvent *);

    VisualController *m_pVisualController;
    Picking m_Picking;

    /** Used in mouse event handler */
    int m_iStartPosX;
    /** Timer id */
    int m_iTimerID;
    /** Backplane */
    VisualBackplane *m_pVisualBackplane;

    Q3PtrList <VisualChannel> m_qlList;
    /** Colors */
    QColor colorBeat, colorSignal, colorHfc, colorMarker, colorFisheye, colorBack, colorCue;

	bool m_painting;
};

#endif
