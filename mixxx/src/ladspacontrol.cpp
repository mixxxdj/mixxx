/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <ladspacontrol.h>

LADSPAControl::LADSPAControl()
{
    m_pBuffer = new LADSPA_Data [LADSPA_MAX_BUFFER_SIZE];
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
    m_Value = value;
    for (int i = 0; i < LADSPA_MAX_BUFFER_SIZE; i++)
    {
        m_pBuffer[i] = value;
    }
}
