/***************************************************************************
                          enginefilterlbh.h  -  description
                             -------------------
    begin                : Thu Apr 4 2002
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

#ifndef ENGINEFILTERLBH_H
#define ENGINEFILTERLBH_H

#include "engineobject.h"
#include "engineiirfilter.h"
#include "mixxxview.h"

/**
  * Parallel processing of LP, BP and HP filters, and final mixing
  *
  *@author Tue and Ken Haste Andersen
  */

class EngineFilterLBH : public EngineObject  {
	Q_OBJECT
public:
	EngineFilterLBH(MixxxView *view, MidiObject *midi);
	~EngineFilterLBH();
	CSAMPLE *process(CSAMPLE *source, int buf_size);
private:
	EngineIIRfilter *low, *band, *high;
	CSAMPLE *buffer;
};

#endif
