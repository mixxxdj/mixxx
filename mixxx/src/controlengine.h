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
#include <qobject.h>
//#include "engineobject.h"
class ControlObject;


/**
  *@author Tue & Ken Haste Andersen
  *
  * This class is used to store a control value as used by an EngineObject.
  * ControlEngine is thread safe, and sends a user event to a ControlObject
  * whenever it's value is changed. setExtern is used for external threads
  * to set it's value.
  */

class ControlEngine : public QObject
{
    Q_OBJECT
public: 
    ControlEngine(ControlObject *_controlObject);
    virtual ~ControlEngine();
    /** An method in an EngineObject is set to be called whenever the value is changed by the
      * external ControlObject. In this way polling on ControlEngine objects can be avoided in
      * the player thread. */
    //void setNotify(EngineObject *, EngineMethod);
    /** Returns the value of the object */
    double get() { return value; };
    /** The value is changed by the engine, and the corresponding ControlObject is updated */
    void set(double v);
    /** Setting the value from an external controller. This happen when a ControlObject has
      * changed and its value is syncronized with this object */
    void setExtern(double v);
    /** Adds a value to the value property of the ControlEngine. Notification in a similar way
      * to set */
    void add(double v);
    /** Subtracts a value to the value property of the ControlEngine. Notification in a similar way
      * to set */
    void sub(double v);

signals:
	void valueChanged(double);

private:
    /** Pointer to corresponding ControlObject */
    ControlObject *controlObject;
    /** The actual value of the object */
    double value;

    /** Set to an EngineObject to notify when control value changes by call to setExtern */
    //EngineObject *notifyobj;
    //void (EngineObject::*notifymethod) (double);
};

#endif
