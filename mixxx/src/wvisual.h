/***************************************************************************
                          wvisual.h  -  description
                             -------------------
    begin                : Thu Oct 10 2002
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

#ifndef WVISUAL_H
#define WVISUAL_H

#include <qgl.h>
#include <qptrlist.h>
#include <qevent.h>
#include <qdatetime.h>

#include "visual/visualcontroller.h"
#include "visual/picking.h"
#include "visual/visualbackplane.h"
#include "visual/visualchannel.h"

class ControlPotmeter;

#include "defs.h"

/**
  *@author Tue & Ken Haste Andersen
  */

class WVisual : public QGLWidget
{
    Q_OBJECT
public: 
    WVisual(QWidget *pParent=0, const char *pName=0, const QGLWidget *pShareWidget = 0, QColor qBackground = QColor(0,0,0));
    ~WVisual();
    bool eventFilter(QObject *o, QEvent *e);
    /** Add a VisualChannel */
    VisualChannel *add(ControlPotmeter *pPlaypos);

public slots:
    void setValue(double) {};
signals:
    void valueChangedLeftDown(double);
    void valueChangedRightDown(double);

protected:
    void initializeGL();
    void resizeGL(int, int);
    void paintGL();
    void timerEvent(QTimerEvent *);

    VisualController *m_pVisualController;
    Picking m_Picking;

    int m_iScreenX;
    int m_iScreenY;
    int m_iSelObjIdx;

    /** Used in mouse event handler */
    int m_iStartPosX;

    /** Backplane */
    VisualBackplane *m_pVisualBackplane;

    QPtrList <VisualChannel> m_qlList;
    QTime m_qtTime;
};

#endif
