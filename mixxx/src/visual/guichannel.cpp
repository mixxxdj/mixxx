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
#include "../reader.h"
#include "../readerevent.h"
#include "visualcontroller.h"
#include "guicontainer.h"
#include "guisignal.h"
#include "signalvertexbuffer.h"

int GUIChannel::channelTotal = 0;

/**
 * Default Consructor.
 */

GUIChannel::GUIChannel(Reader *_reader, ControlPotmeter *_playpos, VisualController *_controller)
{
    channelNo = channelTotal;
    channelTotal++;

    reader = _reader;
    playpos = _playpos;
    controller = _controller;
    list.setAutoDelete(true);

    installEventFilter(this);
}

GUIChannel::~GUIChannel()
{
    channelTotal--;
}

bool GUIChannel::eventFilter(QObject *o, QEvent *e)
{
    // Update buffers
    // If a user events are received, update containers
    if (e->type() == (QEvent::Type)10002)
    {
        ReaderEvent *re = (ReaderEvent *)e;
        GUIContainer *c;
        for (c = list.first(); c; c = list.next())
            c->getBuffer()->update(re->pos(), re->len());
    }
    else
    {
        // standard event processing
        return QObject::eventFilter(o,e);
    }
    return TRUE;
}

void GUIChannel::zoom(int id)
{
    GUIContainer *c;
    for (c = list.first(); c; c = list.next())
        if (id==c->getId())
            c->zoom();
}

SignalVertexBuffer *GUIChannel::add(ReaderExtract *readerExtract)
{
    // Construct a new container
    GUIContainer *c = new GUIContainer(readerExtract, playpos);

    qDebug("pos %i,%i",posx,30*(list.count()));
    // Base y pos dependent on number of containers
    c->setBasepos(posx,30*(list.count()),0);

    // Zoom y pos dependent on channel
    if (channelNo==0)
        c->setZoompos(zoomposx,20,0);
    else
        c->setZoompos(zoomposx,20,10);

    // Append container to list
    list.append(c);

    // Add GUISignal to controller
    controller->add(c->getSignal());

    if (list.count()==1)
        c->zoom();
    
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

