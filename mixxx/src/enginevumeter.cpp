/***************************************************************************
                          enginevumeter.cpp  -  description
                             -------------------
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "enginevumeter.h"
#include "controlengine.h"
#include "controlpotmeter.h"

EngineVUmeter::EngineVUmeter(const char *group)
{
    // The VUmeter widget is controlled via a controlpotmeter, which means
    // that it should react on the setValue(int) signal.
    ControlPotmeter *ctrlVUmeter = new ControlPotmeter(ConfigKey(group, "VUmeter"), 0., 1.);
    m_ctrlVUmeter = new ControlEngine(ctrlVUmeter);
    m_ctrlVUmeter->set(0);

    // Initialize the calculation:
    m_iSamplesCalculated = 0;
    m_fRMSvolume = 0;
}

EngineVUmeter::~EngineVUmeter()
{
    delete m_ctrlVUmeter;
}

CSAMPLE *EngineVUmeter::process(const CSAMPLE *source, const int buffer_size)
{
    // Calculate the summed absolute volume
    for (int i=0; i<buffer_size; i++)
    {
        m_fRMSvolume += abs(source[i]);
    }
    m_iSamplesCalculated += buffer_size;

    // Are we ready to update the VU meter?:
    if (m_iSamplesCalculated > (44100/UPDATE_RATE) )
    {
        m_fRMSvolume = log10(m_fRMSvolume/(m_iSamplesCalculated*1000)+1);
        m_ctrlVUmeter->set( min(1.0, max(0.0, m_fRMSvolume)) ); 
        // Reset calculation:
        m_iSamplesCalculated = 0;
        m_fRMSvolume = 0;
    }

    return 0;
}
