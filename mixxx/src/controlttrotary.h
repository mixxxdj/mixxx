/***************************************************************************
                          controlttrotary.h  -  description
                             -------------------
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
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

#ifndef CONTROLTTROTARY_H
#define CONTROLTTROTARY_H

#include "configobject.h"
#include "controlobject.h"
#include "defs.h"
//#include <qtimer.h>

/** Turn Table rotary controller class. The turntable rotary sends midi events: 0 when turning
  * backwards, and 1 when turning forward. This class keeps track of it's speed, using a timer
  * interrupt */
class ControlTTRotary : public ControlObject
{
    Q_OBJECT
public:
    ControlTTRotary(ConfigKey key);
    FLOAT_TYPE getValue();
    void setValue(int);
    void setAccelUp(const QKeySequence key) {};
    void setAccelDown(const QKeySequence key) {};
public slots:
    void slotSetPositionExtern(float);
    void slotSetPositionMidi(MidiCategory c, int v);
//    void slotSetValue(int newvalue);
protected:
    void forceGUIUpdate();
private:
    /** Decreases/increases received since last timer event */
    int received;    
//    QTimer timer;
private slots:
//    void slotTimer();
};

#endif
