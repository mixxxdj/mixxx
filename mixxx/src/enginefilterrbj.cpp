/***************************************************************************
                          enginefilterrbj.cpp  -  description
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

#include "enginefilterrbj.h"

EngineFilterRBJ::EngineFilterRBJ(int potmeter_midi, int button_midi,
				 int button_bit, MidiObject *midi)
{
	dbGain = 100.;

	filterpot = new ControlPotmeter("filterpot", potmeter_midi, midi);
	connect(filterpot, SIGNAL(valueChanged(FLOAT_TYPE)), this, SLOT(slotUpdate()));

    updateFilter();

	buffer = new CSAMPLE[MAX_BUFFER_LEN];
}

EngineFilterRBJ::~EngineFilterRBJ()
{
	delete [] buffer;
	delete filterpot;
}

CSAMPLE *EngineFilterRBJ::process(const CSAMPLE *source, const int buf_size)
{
    for (int i=0; i<buf_size; i++)
	{
		buffer[i] = c0* source[i] + c1*x1 + c2*x2 - c3*y1 - c4*y2;

        // Shift buffered samples
	    x2 = x1;
        x1 = source[i];
        y2 = y1;
        y1 = buffer[i];
    }
	return buffer;
}

void EngineFilterRBJ::slotUpdate()
{
	dbGain = 10.*filterpot->getValue();
    updateFilter();

	qDebug("Got gain: %f",dbGain);
}

void EngineFilterRBJ::updateFilter()
{
	// Determine filter constants
    CSAMPLE omega = two_pi*frequency/SRATE;
    CSAMPLE sn    = sin(omega);
    CSAMPLE cs    = cos(omega);
    CSAMPLE A     = pow(10, dbGain /40);
    CSAMPLE alpha = sn * sinh(ln_2 /2. * bandwidth * omega /sn);
    CSAMPLE beta  = sqrt(A + A);

    CSAMPLE b0, b1, b2, a0, a1, a2;

	// Low pass filter constants
    b0 = (1.-cs)/2.;
    b1 = 1.-cs;
    b2 = (1.-cs)/2.;
    a0 = 1.+alpha;
    a1 = -2.*cs;
    a2 = 1.-alpha;
	
    c0 = b0 / a0;
    c1 = b1 / a0;
    c2 = b2 / a0;
    c3 = a1 / a0;
    c4 = a2 / a0;

    // Set samples to zero
    x1 = x2 = 0.;
    y1 = y2 = 0.;
}
