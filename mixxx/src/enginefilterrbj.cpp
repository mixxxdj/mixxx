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
	gain = 100.;

	filterpot = new ControlPotmeter("filterpot", potmeter_midi, midi);
	connect(filterpot, SIGNAL(valueChanged(FLOAT_TYPE)), this, SLOT(slotUpdate()));

    updateFilter();
	s0 = 0.; s1 = 0.; s2 = 0.; d0 = 0.; d1 = 0.; d2 = 0.;

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
		s2 = s1;
		s1 = s0;
		s0 = source[i];

		d2 = d1;
		d1 = d0;

		buffer[i] = (b0/a0)*s0 + (b1/a0)*s1 + (b2/a0)*s2 - (a1/a0)*d1 - (a2/a0)*d2;

		d0 = buffer[i];
	}
	return buffer;
}

void EngineFilterRBJ::slotUpdate()
{
	gain = filterpot->getValue();
    updateFilter();

	qDebug("Got gain: %f",gain);
}

void EngineFilterRBJ::updateFilter()
{
	// Determine filter constants
    omega = two_pi*frequency/SRATE;
    sn    = sin(omega);
    cs    = cos(omega);
    alpha = sn/(2.*gain*1000.); //sn*sinh(log(2.)/2. * bandwidth * omega/sn);

	// Low pass filter constants
    b0 = (1.-cs)/2.;
    b1 = 1.-cs;
    b2 = (1.-cs)/2.;
    a0 = 1.+alpha;
    a1 = -2.*cs;
    a2 = 1.-alpha;
	
	qDebug("a0: %f, b0: %f",a0,b0);
}
