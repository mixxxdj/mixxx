/***************************************************************************
                          controlrotary.h  -  description
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

#ifndef CONTROLROTARY_H
#define CONTROLROTARY_H

#include "configobject.h"
#include "controlpotmeter.h"
#include "defs.h"
#include "controlpushbutton.h"
// #include <sys/time.h>
#include <algorithm>

class ControlRotary : public ControlPotmeter
{
    Q_OBJECT
public:
    ControlRotary(ConfigKey key, ControlPushButton *playbutton);
    void updatecounter(int, int SRATE);
    short direction;
    void setValue(int);
    void setAccelUp(const QKeySequence key) {};
    void setAccelDown(const QKeySequence key) {};
public slots:
    void slotSetPositionExtern(float);
    void slotSetPositionMidi(MidiCategory, int);
    void slotSetValue(int newvalue);
protected:
    void forceGUIUpdate();
private:
    FLOAT_TYPE counter;
    //timeval oldtime;
//    static const char graycodetable[256];
    ControlPushButton *play;
};

#endif
