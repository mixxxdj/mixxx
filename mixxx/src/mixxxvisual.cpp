/***************************************************************************
                          mixxxvisual.cpp  -  description
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

#include "mixxxvisual.h"
#include "visual/fastvertexarray.h"
#include "visual/signalvertexbuffer.h"
#include "visual/guisignal.h"
#include "visual/guicontainer.h"

MixxxVisual::MixxxVisual(QWidget *parent, const char *name) : QGLWidget(parent,name)
{
    idCount = 0;
    installEventFilter(this);
    time.start();
    startTimer(0);
}

MixxxVisual::~MixxxVisual()
{
    delete vertex;

    // Delete list
}

bool MixxxVisual::eventFilter(QObject *o, QEvent *e)
{
    // Handle mouse press events
    if (e->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *m = (QMouseEvent *)e;
        int id = picking.pick(m->x(),m->y());
        if (id==1) id=1;
        if (id==2) id=1;
        if (id==3) id=2;
        if (id==4) id=2;
        GUIContainer *c = getContainer(id);
        if (c!=0)
            c->zoom();
    }
    else
    {
        // standard event processing
        return QObject::eventFilter(o,e);
    }
    return TRUE;
}

GUIContainer *MixxxVisual::add(EngineBuffer *engineBuffer)
{
    GUIContainer *c= new GUIContainer(vertex, engineBuffer);

    // Add GUISignal to controller
    controller.add(c->getSignal());

    // Add container to container list
    ContainerEntry *e = new ContainerEntry();
    e->container = c;
    ++idCount; e->id = idCount;

    list.append(e);

    return c;
}

int MixxxVisual::remove(int id)
{
    return 0;
}

GUIContainer *MixxxVisual::getContainer(int id)
{
    ContainerEntry *e;
    for (e = list.first(); e; e = list.next() )
        if (e->id==id)
            return e->container;
    return 0;
}

    
void MixxxVisual::initializeGL()
{
    controller.init();
    backplane = new VisualBackplane();
    controller.add(backplane);
    vertex = new FastVertexArray();
    vertex->init(READCHUNKSIZE/RESAMPLE_FACTOR,READCHUNK_NO*2); // Allocate room for a lot of vertices

    picking.init(&controller);
}


void MixxxVisual::paintGL()
{
    // Get time since last paint, and reset timer
    int msec = time.elapsed();
    time.restart();

    // Update position of each component
    ContainerEntry *e;
    for (e = list.first(); e; e = list.next() )
        e->container->move(msec);

    // Display stuff
    controller.display();
}

void MixxxVisual::resizeGL(int width, int height)
{
    controller.resize((GLsizei)width,(GLsizei)height);
}

void MixxxVisual::timerEvent(QTimerEvent*)
{
    updateGL();
}
