/***************************************************************************
                          guichannel.h  -  description
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
 
#ifndef GUICHANNEL_H
#define GUICHANNEL_H

#include <qobject.h>
#include <qptrlist.h>
#include <qobject.h>
#include <qevent.h>

class GUIContainer;
class Reader;
class ReaderExtract;
class VisualController;
class SignalVertexBuffer;
class ControlPotmeter;

/**
 * A GUI Channel
 * This class keeps track of the GUIContainers associated with one channel
 *
 *
 */
class GUIChannel : public QObject
{
public:
    GUIChannel(Reader *_reader, ControlPotmeter *_playpos, VisualController *_controller);
    ~GUIChannel();
    bool eventFilter(QObject *o, QEvent *e);
    SignalVertexBuffer *add(ReaderExtract *readerExtract);
    void move(int msec);
    void setPosX(int x);
    void setZoomPosX(int x);    
private:
    Reader                  *reader;
    VisualController        *controller;
    ControlPotmeter         *playpos;
    QPtrList<GUIContainer>  list;
    int                     posx, zoomposx;
};
#endif
