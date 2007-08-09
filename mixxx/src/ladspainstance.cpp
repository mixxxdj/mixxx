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
    int sampleRate = (int)m_pControlObjectSampleRate->get();
    qDebug("LADSPA: Sample rate: %d", sampleRate);

    m_Handle = m_pDescriptor->instantiate(m_pDescriptor, sampleRate);

    if (m_pDescriptor->activate)
    {
        m_pDescriptor->activate(m_Handle);
    }

    for (unsigned long port = 0; port < m_pDescriptor->PortCount; port++)
    {
        qDebug("LADSPA: Port %lu: %s", port, m_pDescriptor->PortNames[port]);
        if (LADSPA_IS_PORT_AUDIO (m_pDescriptor->PortDescriptors [port]))
        {
            if (LADSPA_IS_PORT_INPUT (m_pDescriptor->PortDescriptors [port]))
            {
                m_InputPort = port;
            }
            else
            {
                m_OutputPort = port;
            }
        }
    }
    qDebug("LADSPA: Input: %lu", m_InputPort);
    qDebug("LADSPA: Output: %lu", m_OutputPort);
}

LADSPAInstance::~LADSPAInstance()
{
    if (m_pDescriptor->deactivate)
    {
        m_pDescriptor->deactivate(m_Handle);
    }

    m_pDescriptor->cleanup(m_Handle);
}

void LADSPAInstance::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{
    m_pDescriptor->connect_port(m_Handle, m_InputPort, (LADSPA_Data*) pIn);
    m_pDescriptor->connect_port(m_Handle, m_OutputPort, (LADSPA_Data*) pOut);
    m_pDescriptor->run(m_Handle, iBufferSize);
}

void LADSPAInstance::connect(unsigned long port, LADSPA_Data * buffer)
{
    m_pDescriptor->connect_port(m_Handle, port, buffer);
}
