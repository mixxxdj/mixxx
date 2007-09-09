/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <ladspainstance.h>

ControlObject * LADSPAInstance::m_pControlObjectSampleRate = NULL;

LADSPAInstance::LADSPAInstance(const LADSPA_Descriptor * descriptor)
{
    m_pDescriptor = descriptor;

    if (m_pControlObjectSampleRate == NULL)
    {
        m_pControlObjectSampleRate = ControlObject::getControl(ConfigKey("[Master]", "samplerate"));
    }

    if (LADSPA_IS_INPLACE_BROKEN(m_pDescriptor->Properties))
    {
        qDebug("LADSPA: Inplace broken!");
    }

    remove = 0;
}

const LADSPA_Descriptor * LADSPAInstance::getDescriptor()
{
    return m_pDescriptor;
}

int LADSPAInstance::getSampleRate()
{
    return (int)m_pControlObjectSampleRate->get();
}

LADSPAInstance::~LADSPAInstance()
{
}

bool LADSPAInstance::isInplaceBroken()
{
    return LADSPA_IS_INPLACE_BROKEN(getDescriptor()->Properties);
}
