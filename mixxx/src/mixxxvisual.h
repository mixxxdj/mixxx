/***************************************************************************
                          mixxxvisual.h  -  description
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

#ifndef MIXXXVISUAL_H
#define MIXXXVISUAL_H

#include <qgl.h>
#include <qptrlist.h>
#include <qevent.h>
#include <qdatetime.h>
#include <qapplication.h>

#include "visual/visualcontroller.h"
#include "visual/picking.h"
#include "visual/visualbackplane.h"

class EngineBuffer;
class GUIChannel;

const int RESAMPLE_FACTOR = 32;

#include "defs.h"

/**
  *@author Tue & Ken Haste Andersen
  */

class MixxxVisual : public QGLWidget
{
    Q_OBJECT
public: 
    MixxxVisual(QApplication *app, QWidget *parent=0, const char *name=0);
    ~MixxxVisual();
    bool eventFilter(QObject *o, QEvent *e);
    /** Add a GUIChannel */
    GUIChannel *add(EngineBuffer *engineBuffer);
protected:
    void initializeGL();
    void resizeGL(int, int);
    void paintGL();
    void timerEvent(QTimerEvent *);

    VisualController *controller;
    Picking picking;

    int screenx;
    int screeny;
    int selObjIdx;

    /** Backplane */
    VisualBackplane *backplane;

    QPtrList <GUIChannel> list;
    QTime time;
};

#endif
