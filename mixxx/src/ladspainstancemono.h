/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LADSPAINSTANCEMONO_H
#define LADSPAINSTANCEMONO_H

#include <QtCore>
#include <ladspa.h>

#include "defs.h"
#include "controlobject.h"
#include "ladspainstance.h"

class LADSPAInstanceMono : public LADSPAInstance
{
public:
    LADSPAInstanceMono(const LADSPA_Descriptor * descriptor, int slot);
    ~LADSPAInstanceMono();

    void process(const CSAMPLE * pInLeft, const CSAMPLE * pInRight, const CSAMPLE * pOutLeft, const CSAMPLE * pOutRight, const int iBufferSize);
    void connect(unsigned long port, LADSPA_Data * buffer);

private:
    LADSPA_Handle m_HandleLeft;
    LADSPA_Handle m_HandleRight;
    unsigned long m_InputPort;
    unsigned long m_OutputPort;
};

#endif
