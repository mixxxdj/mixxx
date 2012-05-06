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
#include "defs.h"
#include <qdatetime.h>

/**
  * Takes impulses as input, and convert it to a BPM measure.
  *
  *@author Tue & Ken Haste Andersen
  */

/** Minimum allowed Beat per minute (BPM) */
const int minBPM = 30;
/** Maximum allowed bpm */
const int maxBPM = 240;
/** Maximum allowed interval between beats in milli seconds (calculated from minBPM) */
const int maxInterval = (int)(1000.*(60./(CSAMPLE)minBPM));
/** Filter length */
const int filterLength = 5;

class ControlBeat : public ControlObject {
  public:
    ControlBeat(ConfigKey key, bool bMidiSimulateLatching=false);
    virtual ~ControlBeat();

  protected:
    void setValueFromMidi(MidiOpCode o, double v);
    void setValueFromThread(double dValue);
  private:
    void beatTap();

    QTime time;
    CSAMPLE *buffer;
    bool m_bMidiSimulateLatching;
    bool m_bPressed;
    int m_iValidPresses;
};

#endif
