/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "engineladspa.h"

#include "controlpotmeter.h"
#include "ladspacontrol.h"

EngineLADSPA::EngineLADSPA()
{
    m_Loader = new LADSPALoader;
    m_bufferSize = 0;
    LADSPAInstance * instance = m_Loader->getByLabel("delay_5s")->instantiate();
    m_pControl1 = new LADSPAControl;
    m_pControl2 = new LADSPAControl;
    m_pControl3 = new LADSPAControl;
    m_pControl4 = new LADSPAControl;
    this->addInstance(instance);
    instance->connect(0, m_pControl1->getBuffer()); // delay
    instance->connect(1, m_pControl2->getBuffer()); // wet/dry mix
    //instance->connect(2, m_pControl2->getBuffer());
    //instance->connect(3, m_pControl3->getBuffer());
    //m_pControl1->setValue(0.3);
    //m_pControl2->setValue(0.5);
    m_pPot1 = new ControlPotmeter(ConfigKey("[LADSPA]", "control1"), 0., 5.);
    m_pPot2 = new ControlPotmeter(ConfigKey("[LADSPA]", "control2"), 0., 1.);
    m_pPot3 = new ControlPotmeter(ConfigKey("[LADSPA]", "control3"), 0., 1.);
    m_pPot4 = new ControlPotmeter(ConfigKey("[LADSPA]", "control4"), 0., 1.);
    m_pPot2->set(0.);
}

EngineLADSPA::~EngineLADSPA()
{
}

void EngineLADSPA::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize)
{
    if (iBufferSize != m_bufferSize)
    {
        m_bufferSize = iBufferSize;
        qDebug("LADSPA: setBufferSize: %d", m_bufferSize);
        LADSPAControl::setBufferSize(m_bufferSize);
    }

    m_pControl1->setValue(m_pPot1->get());
    m_pControl2->setValue(m_pPot2->get());
    m_pControl3->setValue(m_pPot3->get());
    m_pControl4->setValue(m_pPot4->get());

    // TODO: stereo
    for (LADSPAInstanceList::iterator instance = m_Instances.begin(); instance != m_Instances.end(); instance++)
    {
        if (instance == m_Instances.begin())
        {
            (*instance)->process(pIn, pOut, iBufferSize);
        }
        else
        {
            (*instance)->process(pOut, pOut, iBufferSize); // TODO: fix for inplace broken plugins
        }
    }
}

void EngineLADSPA::addInstance(LADSPAInstance * instance)
{
    m_Instances.append(instance);
}
