/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtCore>
#include "ladspainstancemono.h"

LADSPAInstanceMono::LADSPAInstanceMono(const LADSPA_Descriptor * descriptor, int slot) : LADSPAInstance(descriptor, slot)
{
    int sampleRate = getSampleRate();
    //qDebug() << "LADSPA: Sample rate: " << sampleRate;

    m_HandleLeft = descriptor->instantiate(descriptor, sampleRate);
    m_HandleRight = descriptor->instantiate(descriptor, sampleRate);

    if (descriptor->activate)
    {
        descriptor->activate(m_HandleLeft);
        descriptor->activate(m_HandleRight);
    }

    m_InputPort = descriptor->PortCount;
    m_OutputPort = descriptor->PortCount;
    for (unsigned long port = 0; port < descriptor->PortCount; port++)
    {
        //qDebug() << "LADSPA: Port " << port << "u: " << descriptor->PortNames[port];
        if (LADSPA_IS_PORT_AUDIO(descriptor->PortDescriptors [port]))
        {
            if (LADSPA_IS_PORT_INPUT(descriptor->PortDescriptors [port]))
            {
                m_InputPort = port;
            }
            else
            {
                m_OutputPort = port;
            }
        } else {
        	//qDebug() << "LADSPA: Port" << "Range:" <<  descriptor->PortRangeHints[port].LowerBound << "~" <<  descriptor->PortRangeHints[port].UpperBound << "isInteger:" << LADSPA_IS_HINT_INTEGER(descriptor->PortRangeHints[port].HintDescriptor);
        }
    }
    //qDebug() << "LADSPA: Input: " << m_InputPort << "u";
    //qDebug() << "LADSPA: Output: " << m_OutputPort << "u";
}

LADSPAInstanceMono::~LADSPAInstanceMono()
{
    if (getDescriptor()->deactivate)
    {
        getDescriptor()->deactivate(m_HandleLeft);
        getDescriptor()->deactivate(m_HandleRight);
    }

    getDescriptor()->cleanup(m_HandleLeft);
    getDescriptor()->cleanup(m_HandleRight);
}

void LADSPAInstanceMono::process(const CSAMPLE * pInLeft, const CSAMPLE * pInRight, const CSAMPLE * pOutLeft, const CSAMPLE * pOutRight, const int iBufferSize)
{
    getDescriptor()->connect_port(m_HandleLeft, m_InputPort, (LADSPA_Data *) pInLeft);
    getDescriptor()->connect_port(m_HandleLeft, m_OutputPort, (LADSPA_Data *) pOutLeft);
    getDescriptor()->connect_port(m_HandleRight, m_InputPort, (LADSPA_Data *) pInRight);
    getDescriptor()->connect_port(m_HandleRight, m_OutputPort, (LADSPA_Data *) pOutRight);
    getDescriptor()->run(m_HandleLeft, iBufferSize);
    getDescriptor()->run(m_HandleRight, iBufferSize);
}

void LADSPAInstanceMono::connect(unsigned long port, LADSPA_Data * buffer)
{
    getDescriptor()->connect_port(m_HandleLeft, port, buffer);
    getDescriptor()->connect_port(m_HandleRight, port, buffer);
}
