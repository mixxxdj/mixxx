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

#define LADSPA_MAX_BUFFER_SIZE 1024

#include <ladspa.h>

class LADSPAControl {

public:
    LADSPAControl();
    ~LADSPAControl();

    LADSPA_Data * getBuffer();
    void setValue(LADSPA_Data value);
    
protected:
    LADSPA_Data m_Value;
    LADSPA_Data * m_pBuffer;
};

#endif
