/***************************************************************************
                          guichannel.cpp  -  description
                             -------------------
    copyright            : (C) 2003 by Tue and Ken Haste Andersen and Kenny
                                       Erleben
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "guichannel.h"
#include "../defs.h"
#include "../enginebuffer.h"
#include "../soundbuffer.h"
#include "visualcontroller.h"
#include "guicontainer.h"
#include "guisignal.h"
#include "signalvertexbuffer.h"

/**
 * Default Consructor.
 */

GUIChannel::GUIChannel(EngineBuffer *_engineBuffer, VisualController *_controller)
{
    engineBuffer = _engineBuffer;
    controller = _controller;
    list.setAutoDelete(true);

    installEventFilter(this);
}

GUIChannel::~GUIChannel()
{
}

bool GUIChannel::eventFilter(QObject *o, QEvent *e)
{
    // If a user events are received, update containers
    if (e->type() == (QEvent::Type)1001)
    {
        GUIContainer *c;
        for (c = list.first(); c; c = list.next())
            c->update();
    }
    else
    {
        // standard event processing
        return QObject::eventFilter(o,e);
    }
    return TRUE;
}


SignalVertexBuffer *GUIChannel::add(SoundBuffer *soundBuffer)
{
    // Construct a new container
    GUIContainer *c = new GUIContainer(engineBuffer);

    if (list.isEmpty())
    {
        c->setBasepos(posx,32,0);
        c->setZoompos(zoomposx,20,0);
    }
    else
    {
        c->setBasepos(posx,32,0);
        c->setZoompos(zoomposx,-10,0);
    }

    // Add GUISignal to controller
    controller->add(c->getSignal());

    // Append container to list
    list.append(c);

    return c->getBuffer();
}

void GUIChannel::move(int msec)
{
    GUIContainer *c;
    for (c = list.first(); c; c = list.next())
        c->move(msec);
}

void GUIChannel::setPosX(int x)
{
    posx = x;
}

void GUIChannel::setZoomPosX(int x)
{
    zoomposx = x;
}

