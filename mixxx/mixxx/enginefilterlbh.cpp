/***************************************************************************
                          enginefilterlbh.cpp  -  description
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

#include "enginefilterlbh.h"
#include "qknob.h"

EngineFilterLBH::EngineFilterLBH(MixxxView *view, MidiObject *midi)
{
	low = new EngineIIRfilter(ADC6,PORT_B, 4, midi,bessel_lowpass);
	connect(view->channel->DialFilterLow,  SIGNAL(valueChanged(int)), low->filterpot,  SLOT(slotSetPosition(int)));
	band = new EngineIIRfilter(ADC5,PORT_B, 4, midi,bessel_bandpass);
	connect(view->channel->DialFilterMiddle,  SIGNAL(valueChanged(int)), band->filterpot,  SLOT(slotSetPosition(int)));
	high = new EngineIIRfilter(ADC4,PORT_B, 4, midi,bessel_highpass);
	connect(view->channel->DialFilterHigh,  SIGNAL(valueChanged(int)), high->filterpot,  SLOT(slotSetPosition(int)));

	buffer = new CSAMPLE[MAX_BUFFER_LEN];
}

EngineFilterLBH::~EngineFilterLBH()
{
	delete [] buffer;
	delete high;
	delete band;
	delete low;
}

CSAMPLE *EngineFilterLBH::process(CSAMPLE *source, int buf_size)
{
	CSAMPLE *p0 = low->process(source,buf_size);
	CSAMPLE *p1 = band->process(source,buf_size);
	CSAMPLE *p2 = high->process(source,buf_size);

	for (int i=0; i<buf_size; i++)
		buffer[i] = p0[i]+p1[i]+p2[i];

	return buffer;
}








