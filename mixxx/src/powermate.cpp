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

PowerMate::PowerMate()
{
    m_pControlObjectRotary = 0;
    m_pControlObjectButton = 0;

    m_bSendKnobEvent = false;
    m_fMagnitude=0.;

    m_pKnobIntegral = new int[kiPowermateKnobIntegralMaxLen];
    for (int i=0; i<kiPowermateKnobIntegralMaxLen; i++)
        m_pKnobIntegral[i] = 0;
    m_iKnobIntegralLength = kiPowermateKnobIntegralMaxLen;

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

void PowerMate::selectMapping(QString mapping)
{
    if (mapping==kqPowerMateMappingP1Phase)
    {
        m_pControlObjectRotary = ControlObject::getControl(ConfigKey("[Channel1]","wheel"));
        m_pControlObjectButton = ControlObject::getControl(ConfigKey("[Channel1]","play"));
        m_iKnobIntegralLength = kiPowermateKnobIntegralMaxLen;
    }
    else if (mapping==kqPowerMateMappingP2Phase)
    {
        m_pControlObjectRotary = ControlObject::getControl(ConfigKey("[Channel2]","wheel"));
        m_pControlObjectRotary = ControlObject::getControl(ConfigKey("[Channel2]","play"));
        m_iKnobIntegralLength = kiPowermateKnobIntegralMaxLen;
    }
    else if (mapping==kqPowerMateMappingP1Scratch)
    {
        m_pControlObjectRotary = ControlObject::getControl(ConfigKey("[Channel1]","scratch"));
        m_pControlObjectButton = ControlObject::getControl(ConfigKey("[Channel1]","play"));
        m_iKnobIntegralLength = 5;
    }
    else if (mapping==kqPowerMateMappingP2Scratch)
    {
        m_pControlObjectRotary = ControlObject::getControl(ConfigKey("[Channel2]","scratch"));
        m_pControlObjectRotary = ControlObject::getControl(ConfigKey("[Channel2]","play"));
        m_iKnobIntegralLength = 5;
    }
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
    for (int i=0; i<m_iKnobIntegralLength-1; i++)
    {
        m_pKnobIntegral[i]=m_pKnobIntegral[i+1];
        m_fMagnitude += m_pKnobIntegral[i];
        if (m_pKnobIntegral[i]!=0)
            bStop = false;
    }
    m_pKnobIntegral[m_iKnobIntegralLength-1] = m_iKnobVal;
    m_fMagnitude += m_pKnobIntegral[m_iKnobIntegralLength-1];

    m_fMagnitude = 25.*m_fMagnitude/m_iKnobIntegralLength;
    qDebug("mag %f",m_fMagnitude);

    // Range check
    if (m_fMagnitude>63)
        m_fMagnitude = 63;
    else if (m_fMagnitude<-64)
        m_fMagnitude = -64;

    // Post event
    if (m_pControlObjectRotary)
        m_pControlObjectRotary->queueFromThread((m_fMagnitude)/127.);

    if (bStop && m_fMagnitude==0)
        m_bSendKnobEvent = false;

    // Reset knob value
    m_iKnobVal = 0;
}


