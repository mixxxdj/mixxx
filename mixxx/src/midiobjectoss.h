/***************************************************************************
                          midiobjectoss.h  -  description
                             -------------------
    begin                : Thu Jul 4 2002
    copyright            : (C) 2002 by Tue & Ken Haste Andersen
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

#ifndef MIDIOBJECTOSS_H
#define MIDIOBJECTOSS_H

#include "midiobject.h"
#include <sys/select.h>

/**
  *@author Tue & Ken Haste Andersen
  */

class MidiObjectOSS : public MidiObject
{
public:
    MidiObjectOSS(QString device);
    ~MidiObjectOSS();
    void devOpen(QString device);
    void devClose();
protected:
    void run();
    void stop();

    int handle;
    unsigned char *buffer;
    fd_set fdset;
};

#endif
