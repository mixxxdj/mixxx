/***************************************************************************
                          powermate.cpp  -  description
                             -------------------
    begin                : Tue Apr 29 2003
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

#include "powermate.h"
#include "controlobject.h"
#include "controleventmidi.h"
#include "qapplication.h"
#include "midiobject.h"
#include "mathstuff.h"

PowerMate::PowerMate(ControlObject *pControl)
{
    m_pControl = pControl;
    m_bSendKnobEvent = false;
    m_fMagnitude=0.;

    m_pKnobIntegral = new int[kiPowermateKnobIntegralLen];
    for (int i=0; i<kiPowermateKnobIntegralLen; i++)
        m_pKnobIntegral[i] = 0;

    m_pRequestLed = new QSemaphore(5);
}

PowerMate::~PowerMate()
{
	if (running())
	{
		terminate();
		wait();
	}
    delete [] m_pKnobIntegral;
    delete m_pRequestLed;
}

void PowerMate::led()
{
    m_pRequestLed->tryAccess(1);
}

void PowerMate::knob_event()
{
    // Move everything one step backwards in integral buffer
    m_fMagnitude = 0.;
    bool bStop = true;
    for (int i=0; i<kiPowermateKnobIntegralLen-1; i++)
    {
        m_pKnobIntegral[i]=m_pKnobIntegral[i+1];
        m_fMagnitude += m_pKnobIntegral[i];
        if (m_pKnobIntegral[i]!=0)
            bStop = false;
    }
    m_pKnobIntegral[kiPowermateKnobIntegralLen-1] = m_iKnobVal;
    m_fMagnitude += m_pKnobIntegral[kiPowermateKnobIntegralLen-1];

    // Range check
    if (m_fMagnitude>63)
        m_fMagnitude = 63;
    else if (m_fMagnitude<-64)
        m_fMagnitude = -64;

    // Post event
    qDebug("knobval: %i, magnitude: %f, midi ctrl: %i, instno: %i",m_iKnobVal,m_fMagnitude,m_iInstNo*2,m_iInstNo);
    QApplication::postEvent(m_pControl,new ControlEventMidi(CTRL_CHANGE, kiPowermateMidiChannel, (char)(m_iInstNo*2+kiPowermateMidiDial),(char)((int)(m_fMagnitude)+64)));

    if (bStop && m_fMagnitude==0)
        m_bSendKnobEvent = false;

    // Reset knob value
    m_iKnobVal = 0;
}


