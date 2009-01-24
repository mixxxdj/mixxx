/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LADSPACONTROL_H
#define LADSPACONTROL_H

// won't be used unless LADSPAControl is instantiated before a call to setBufferSize
#define LADSPA_MAX_BUFFER_SIZE 8192

#include <ladspa.h>

class LADSPAControl
{
public:
    LADSPAControl();
    ~LADSPAControl();

    LADSPA_Data * getBuffer();
    void setValue(LADSPA_Data value);
    static void setBufferSize(int size);
    
private:
    LADSPA_Data m_Value;
    LADSPA_Data * m_pBuffer;
    static int m_bufferSize;
};

#endif
