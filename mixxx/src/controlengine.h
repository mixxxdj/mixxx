/***************************************************************************
                          controlengine.h  -  description
                             -------------------
    begin                : Thu Feb 20 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
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

#ifndef CONTROLENGINE_H
#define CONTROLENGINE_H

#include <qmutex.h>
#include <qptrlist.h>
#include "engineobject.h"
class ControlObject;


typedef void(EngineObject::* EngineMethod)(double);


/**
  *@author Tue & Ken Haste Andersen
  *
  * This class is used to store a control value as used by an EngineObject.
  * ControlEngine is thread safe, and sends a user event to a ControlObject
  * whenever it's value is changed. setExtern is used for external threads
  * to set it's value, without triggering an event.
  */

class ControlEngine
{
public: 
    ControlEngine(ControlObject *_controlObject);
    virtual ~ControlEngine();
    QPtrList<ControlEngine> *getList();

    void setNotify(EngineObject *, EngineMethod);
            
    double get();
    void set(double v);
    void setExtern(double v);
    void add(double v);
    void sub(double v);
    
private:
    ControlObject *controlObject;
    static QPtrList<ControlEngine> *list;
    double value;
    /** Set to an EngineObject to notify when control value changes by call to setExtern */
    EngineObject *notifyobj;
    void (EngineObject::*notifymethod) (double);
};

#endif
