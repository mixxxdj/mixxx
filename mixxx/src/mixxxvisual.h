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

#include "visual/visualcontroller.h"
#include "visual/picking.h"
#include "visual/visualbackplane.h"

class FastVertexArray;
class EngineBuffer;
class GUIContainer;

const int RESAMPLE_FACTOR = 42;

#include "defs.h"

/**
  *@author Tue & Ken Haste Andersen
  */

class MixxxVisual : public QGLWidget
{
    Q_OBJECT
public: 
    MixxxVisual(QWidget *parent=0, const char *name=0);
    ~MixxxVisual();

    bool eventFilter(QObject *o, QEvent *e);

	/** Add a visual, and return a unique id */
	GUIContainer *add(EngineBuffer *engineBuffer);
	/** Removes a visual based on its unique id. Returns 0 on success */
	int remove(int id);
protected:
    GUIContainer *getContainer(int id);
    void initializeGL();
    void resizeGL( int, int );
    void paintGL();
    void timerEvent( QTimerEvent * );

    VisualController controller;

    Picking picking;

    int screenx;
    int screeny;
    int selObjIdx;

    /** Vertex buffer */
    FastVertexArray *vertex;

    VisualBackplane *backplane;

    typedef struct {
        int          id;
        GUIContainer *container;
    } ContainerEntry;

    int idCount;
    QPtrList <ContainerEntry> list;
    QTime time;
};

#endif
