/***************************************************************************
                          controlbeat.cpp  -  description
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

#include "controlbeat.h"

ControlBeat::ControlBeat(ConfigKey key, bool bMidiSimulateLatching) : ControlObject(key)
{
    m_bMidiSimulateLatching = bMidiSimulateLatching;
    m_dValue = 0.;
    time.start();
    m_bPressed = false;

    // Filter buffer
    buffer = new CSAMPLE[filterLength];
    for (int i=0; i<filterLength; i++)
        buffer[i] = 0.;
}

ControlBeat::~ControlBeat()
{
    delete [] buffer;
}

void ControlBeat::setValueFromMidi(MidiCategory, int v)
{
    if (!m_bPressed || !m_bMidiSimulateLatching)
    {
        setValue((double)v);
        m_bPressed = true;
    }
    else
        m_bPressed = false;
}

void ControlBeat::setValue(double)
{
    int elapsed = time.elapsed();
    time.restart();

    if (elapsed<=maxInterval)
    {
        // Move back in filter one sample
        int i;
        for (i=1; i<filterLength; i++)
            buffer[i-1] = buffer[i];

        buffer[filterLength-1] = 1000.*(60./elapsed);
        if (buffer[filterLength-1]>maxBPM)
            buffer[filterLength-1] = maxBPM;

        double temp = 0.;
        for (i=0; i<filterLength; i++)
            temp += buffer[i];
        temp /= filterLength;
        m_dValue = temp;
    }
}
