/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtCore>
#include "ladspacontrol.h"

// static variable
int LADSPAControl::m_bufferSize = LADSPA_MAX_BUFFER_SIZE;

LADSPAControl::LADSPAControl()
{
    m_pBuffer = new LADSPA_Data [m_bufferSize];
}

LADSPAControl::~LADSPAControl()
{
    delete [] m_pBuffer;
}

LADSPA_Data * LADSPAControl::getBuffer()
{
    return m_pBuffer;
}

void LADSPAControl::setValue(LADSPA_Data value)
{
#ifdef __LADSPA_SIMPLE_CONTROL__
    m_Value = value;
#elif __FXUNITS__
    // Different instances might share the same LADSPAControl.
    m_Value = value;
#else
    if (m_Value != value)
    {
        // value has changed
        LADSPA_Data step = (value - m_Value) / m_bufferSize;
        m_Value -= step;
        // phase 1: fill the buffer with smoothly changing values
        for (int i = 0; i < m_bufferSize; i++)
        {
            m_Value += step;
            m_pBuffer[i] = m_Value;
        }
    }
    else if (m_pBuffer[0] != value)
#endif
    {
        // phase 2: the buffer is not filled with constant values yet
        for (int i = 0; i < m_bufferSize; i++)
        {
            m_pBuffer[i] = m_Value;
        }
    }
}

void LADSPAControl::setBufferSize(int size)
{
    // FIX: may crash after changing buffer size from smaller to bigger
    m_bufferSize = size;
}
