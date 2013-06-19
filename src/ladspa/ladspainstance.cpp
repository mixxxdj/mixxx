/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtCore>
#include "ladspainstance.h"
#include "controlpushbutton.h"

ControlObjectThreadMain *LADSPAInstance::m_pControlObjectSampleRate = NULL;

LADSPAInstance::LADSPAInstance(const LADSPA_Descriptor * descriptor, int slot)
{
    m_pDescriptor = descriptor;

    if (m_pControlObjectSampleRate == NULL)
    {
        m_pControlObjectSampleRate = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Master]", "samplerate")));
    }

    if (LADSPA_IS_INPLACE_BROKEN(m_pDescriptor->Properties))
    {
        qDebug() << "LADSPA: Inplace broken!";
    }

    remove = 0;

    QString slotString;
    slotString.setNum(slot);
    ConfigKey * key = new ConfigKey("[LADSPA]", "EnableEffect" + slotString);
    m_pControlObjectEnable = ControlObject::getControl(*key);
    key = new ConfigKey("[LADSPA]", "DryWet" + slotString);
    m_pControlObjectDryWet = ControlObject::getControl(*key);
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

bool LADSPAInstance::isEnabled()
{
//    qDebug() << "isEnabled() == " << m_pControlObjectEnable->get();
    return m_pControlObjectEnable->get() > 0.0;
}

float LADSPAInstance::getWet()
{
    return m_pControlObjectDryWet->get();
}
