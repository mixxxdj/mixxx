/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtCore>
#include "ladspaplugin.h"
#include "ladspainstancestereo.h"
#include "ladspainstancemono.h"

LADSPAPlugin::LADSPAPlugin(const LADSPA_Descriptor * descriptor)
{
	m_pDescriptor = descriptor;
}

LADSPAPlugin::~LADSPAPlugin()
{
    // TODO
}

LADSPAInstance * LADSPAPlugin::instantiate(int slot)
{
    int inputs = 0;
    int outputs = 0;
    for (unsigned long port = 0; port < m_pDescriptor->PortCount; port++)
    {
        if (LADSPA_IS_PORT_AUDIO(m_pDescriptor->PortDescriptors [port]))
        {
            if (LADSPA_IS_PORT_INPUT(m_pDescriptor->PortDescriptors [port]))
            {
                inputs++;
            }
            else
            {
                outputs++;
            }
        }
    }

    if (inputs == 2)
    {
        if (outputs == 2)
        {
            return new LADSPAInstanceStereo(m_pDescriptor, slot);
        }
        else if (outputs == 1)
        {
            qDebug() << "LADSPA: 2 inputs + 1 output not supported yet!";
        }
        else
        {
            qDebug() << "LADSPA: unsupported number of outputs (" << outputs << ")!";
        }
    }
    else if (inputs == 1)
    {
        if (outputs == 1)
        {
            return new LADSPAInstanceMono(m_pDescriptor, slot);
        }
        else if (outputs == 2)
        {
            qDebug() << "LADSPA: 1 input + 2 outputs not supported yet!";
        }
        else
        {
            qDebug() << "LADSPA: unsupported number of outputs (" << outputs << ")!";
        }
    }
    else
    {
        qDebug() << "LADSPA: unsupported number of inputs (" << inputs << ")!";
    }

    return NULL;
}

const QString LADSPAPlugin::getLabel()
{
  return QString(m_pDescriptor->Label);
}

const LADSPA_Descriptor * LADSPAPlugin::getDescriptor(){
	return m_pDescriptor;
}

bool LADSPAPlugin::isSupported(){
    int inputs = 0;
    int outputs = 0;
    for (unsigned long port = 0; port < m_pDescriptor->PortCount; port++)
    {
        if (LADSPA_IS_PORT_AUDIO(m_pDescriptor->PortDescriptors [port]))
        {
            if (LADSPA_IS_PORT_INPUT(m_pDescriptor->PortDescriptors [port]))
            {
                inputs++;
            }
            else
            {
                outputs++;
            }
        }
    }

    if ((inputs == 2 && outputs == 2) || (inputs == 1 && outputs == 1)){
    	return true;
    } else {
    	return false;
    }
}
