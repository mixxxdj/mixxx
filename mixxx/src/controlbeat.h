/***************************************************************************
                          controlbeat.h  -  description
                             -------------------
    begin                : Mon Apr 7 2003
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

#ifndef CONTROLBEAT_H
#define CONTROLBEAT_H

#include "controlobject.h"
#include "configobject.h"
#include <qdatetime.h>
#include "defs.h"

/**
  * Takes impulses as input, and convert it to a BPM measure.
  *
  *@author Tue & Ken Haste Andersen
  */

/** Minimum allowed Beat per minute (BPM) */
const int minBPM = 10;
/** Maximum allowed bpm */
const int maxBPM = 240;
/** Maximum allowed interval between beats in milli seconds (calculated from minBPM) */
const int maxInterval = (int)(1000.*(60./(CSAMPLE)minBPM));



/** Filter length */
const int filterLength = 20;
  
class ControlBeat : public ControlObject
{
public: 
    ControlBeat(ConfigKey key);
    ~ControlBeat();
public slots:
    void slotSetPosition(int pos);
protected:
    void forceGUIUpdate();
private:
    QTime time;
    CSAMPLE *buffer;
};

#endif
