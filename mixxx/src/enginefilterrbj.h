/***************************************************************************
                          enginefilterrbj.h  -  description
                             -------------------
    begin                : Wed Apr 3 2002
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

#ifndef ENGINEFILTERRBJ_H
#define ENGINEFILTERRBJ_H

#include "engineobject.h"
#include "midiobject.h"
#include "controlpotmeter.h"
#include "controlpushbutton.h"

const CSAMPLE frequency = 1000.;
const CSAMPLE bandwidth = 10.;

/**
  *@author Tue and Ken Haste Andersen
  */

class EngineFilterRBJ : public EngineObject {
	Q_OBJECT
public:
	EngineFilterRBJ(int potmeter_midi, int button_midi,
				 	int button_bit, MidiObject *midi);
	~EngineFilterRBJ();
	CSAMPLE *process(const CSAMPLE *source, const int buf_size);

	ControlPotmeter* filterpot;
public slots:
	void slotUpdate();
private:
	void updateFilter();

    CSAMPLE c0, c1, c2, c3, c4;

	/** Block boundary values */
    CSAMPLE x1, x2, y1, y2;

	/** Filter gain. Controled via signals/slots */
	CSAMPLE dbGain;

	CSAMPLE *buffer;
};

#endif
