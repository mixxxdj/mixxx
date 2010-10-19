/***************************************************************************
                          input.h  -  description
                             -------------------
    begin                : Thu Feb 24 2005
    copyright            : (C) 2005 by Tue Haste Andersen
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

#ifndef INPUT_H
#define INPUT_H

#include <qthread.h>
#include <qmutex.h>
#include <qstringlist.h>

/**
  * Virtual class for handling event based input devices. This is implemented as a separate thread.
  *
  *@author Tue Haste Andersen
  */

class ControlObject;
class ControlObjectThread;

static QString kqInputMappingPositionP1 = "Player 1, position";
static QString kqInputMappingSongP1     = "Player 1, song selection";
static QString kqInputMappingPositionP2 = "Player 2, position";
static QString kqInputMappingSongP2     = "Player 2, song selection";

class Input : public QThread
{
public:
    Input();
    ~Input();

    /** Open a rotary device */
    virtual bool opendev(QString name) = 0;
    /** Close a rotary device */
    virtual void closedev() = 0;
    /** Wait for the next rotary event. Blocking call. */
    virtual void getNextEvent() = 0;
    /** Return a list of available mappings */
    //virtual QStringList getMappings();
    /** Select mapping */
    //virtual void setMapping(QString mapping);

protected:
    /** Main thread loop */
    virtual void run();
    /** Send out an event */
    void sendEvent(double dValue, ControlObject *pControlObject);
    void sendEvent(double dValue, ControlObjectThread *pControlObjThread);
    /** Send out a button event */
    void sendButtonEvent(bool bPressed, ControlObject *pControlObject);
    void sendButtonEvent(bool bPressed, ControlObjectThread *pControlObjThread);
};

#endif
