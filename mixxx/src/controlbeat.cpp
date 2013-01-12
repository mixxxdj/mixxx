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
    m_iValidPresses = 0;

    // Filter buffer
    buffer = new CSAMPLE[filterLength];
    for (int i=0; i<filterLength; i++)
        buffer[i] = 0.;
}

ControlBeat::~ControlBeat()
{
    delete [] buffer;
}

void ControlBeat::setValueFromMidi(MidiOpCode o, double v)
{
    Q_UNUSED(o);
    Q_UNUSED(v);
    if (!m_bPressed || !m_bMidiSimulateLatching)
    {
        beatTap();
        m_bPressed = true;
    }
    else
        m_bPressed = false;
}

void ControlBeat::setValueFromThread(double dValue) {
    if (dValue > 0) {
        beatTap();
    }
}

void ControlBeat::beatTap()
{
    int elapsed = time.restart();

    if (elapsed <= maxInterval) {
        // Move back in filter one sample
        for (int i = filterLength-1; i > 0; i--)
            buffer[i] = buffer[i-1];

        buffer[0] = 1000.*(60./elapsed);
        if (buffer[0] > maxBPM)
            buffer[0] = maxBPM;

        m_iValidPresses++;
        if (m_iValidPresses > filterLength)
            m_iValidPresses = filterLength;

        double temp = 0.;
        for (int i = 0; i < m_iValidPresses; ++i)
            temp += buffer[i];
        temp /= m_iValidPresses;
        m_dValue = temp;

        emit(valueChanged(m_dValue));
    } else {
        m_iValidPresses = 0;
    }
}
