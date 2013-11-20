/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LADSPAINSTANCESTEREO_H
#define LADSPAINSTANCESTEREO_H

#include <QtCore>

#include <ladspa.h>

#include "defs.h"
#include "controlobject.h"

#include "ladspainstance.h"

class LADSPAInstanceStereo : public LADSPAInstance
{
public:
    LADSPAInstanceStereo(const LADSPA_Descriptor * descriptor, int slot);
    ~LADSPAInstanceStereo();

    void process(const CSAMPLE * pInLeft, const CSAMPLE * pInRight, const CSAMPLE * pOutLeft, const CSAMPLE * pOutRight, const int iBufferSize);
    void connect(unsigned long port, LADSPA_Data * buffer);

private:
    LADSPA_Handle m_Handle;
    unsigned long m_InputPortLeft;
    unsigned long m_OutputPortLeft;
    unsigned long m_InputPortRight;
    unsigned long m_OutputPortRight;
};

#endif
