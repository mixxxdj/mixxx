/***************************************************************************
                          midiobjectnull.h  -  description
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

#ifndef MIDIOBJECTNULL_H
#define MIDIOBJECTNULL_H

#include "midiobject.h"

/**
  *@author Tue & Ken Haste Andersen
  */

class MidiObjectNull : public MidiObject  {
public: 
    MidiObjectNull(QString device);
    ~MidiObjectNull();
    void devOpen(QString device);
    void devClose();
};

#endif
