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
    LADSPAInstance * instance = m_Loader->getByLabel("delay_5s")->instantiate();
    delayControl = new LADSPAControl;
    wetControl = new LADSPAControl;
    this->addInstance(instance);
    instance->connect(0, delayControl->getBuffer());
    instance->connect(1, wetControl->getBuffer());
    delayControl->setValue(0.3);
    wetControl->setValue(0.5);
    m_pPot1 = new ControlPotmeter(ConfigKey("[LADSPA]", "delayTime"), 0., 1.);
}

EngineLADSPA::~EngineLADSPA()
{
}

void EngineLADSPA::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize)
{
    delayControl->setValue(m_pPot1->get());
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
