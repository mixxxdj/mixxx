/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LADSPAINSTANCE_H
#define LADSPAINSTANCE_H

#include <Q3PtrList>
#include <Q3PtrVector>

#include <ladspa.h>

#include <defs.h>
#include <controlobject.h>

class LADSPAInstance
{
public:
    LADSPAInstance(const LADSPA_Descriptor * descriptor);
    ~LADSPAInstance();

    virtual void process(const CSAMPLE * pInLeft, const CSAMPLE * pInRight, const CSAMPLE * pOutLeft, const CSAMPLE * pOutRight, const int iBufferSize) = 0;
    virtual void connect(unsigned long port, LADSPA_Data * buffer) = 0;

    const LADSPA_Descriptor * getDescriptor();
    int getSampleRate();
    bool isInplaceBroken();

    bool remove;

private:
    const LADSPA_Descriptor * m_pDescriptor;
    static ControlObject *m_pControlObjectSampleRate;
};

typedef Q3PtrList<LADSPAInstance> LADSPAInstanceList;
typedef Q3PtrVector<LADSPAInstance> LADSPAInstanceVector;

#endif
