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
    m_pBufferLeft[0] = NULL;
    
    m_pControl1 = new LADSPAControl;
    m_pControl2 = new LADSPAControl;
    m_pControl3 = new LADSPAControl;
    m_pControl4 = new LADSPAControl;
    
    LADSPAPlugin * plugin = m_Loader->getByLabel("dj_eq");
    if (plugin != NULL)
    {
        LADSPAInstance * instance = plugin->instantiate();
        this->addInstance(instance);
        instance->connect(0, m_pControl1->getBuffer()); // low
        instance->connect(1, m_pControl2->getBuffer()); // mid
        instance->connect(2, m_pControl3->getBuffer()); // high
        instance->connect(7, m_pControl4->getBuffer()); // latency out
        m_pPot1 = new ControlPotmeter(ConfigKey("[LADSPA]", "control1"), -70., 6.);
        m_pPot2 = new ControlPotmeter(ConfigKey("[LADSPA]", "control2"), -70., 6.);
        m_pPot3 = new ControlPotmeter(ConfigKey("[LADSPA]", "control3"), -70., 6.);
        m_pPot4 = new ControlPotmeter(ConfigKey("[LADSPA]", "control4"), 0., 1.);
        m_pPot1->set(0.);
        m_pPot2->set(0.);
        m_pPot3->set(0.);
    }
    else
    {
        qDebug("LADSPA: Plugin 'dj_eq' not found!");
        plugin = m_Loader->getByLabel("delay_5s");
        if (plugin == NULL)
        {
            qDebug("LADSPA: Plugin 'delay_5s' not found!");
            return;
        }
        LADSPAInstance * instance = plugin->instantiate();
        this->addInstance(instance);
        instance->connect(0, m_pControl1->getBuffer()); // delay
        instance->connect(1, m_pControl2->getBuffer()); // wet/dry mix
        //instance->connect(2, m_pControl3->getBuffer());
        //instance->connect(3, m_pControl4->getBuffer());
        m_pPot1 = new ControlPotmeter(ConfigKey("[LADSPA]", "control1"), 0., 5.);
        m_pPot2 = new ControlPotmeter(ConfigKey("[LADSPA]", "control2"), 0., 1.);
        m_pPot3 = new ControlPotmeter(ConfigKey("[LADSPA]", "control3"), 0., 1.);
        m_pPot4 = new ControlPotmeter(ConfigKey("[LADSPA]", "control4"), 0., 1.);
        m_pPot1->set(1.);
        m_pPot2->set(0.);
    }
}

EngineLADSPA::~EngineLADSPA()
{
}

void EngineLADSPA::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize)
{
    if (iBufferSize != m_bufferSize)
    {
        m_bufferSize = iBufferSize;
        m_monoBufferSize = m_bufferSize / 2;
        qDebug("LADSPA: setBufferSize: %d (%d)", m_monoBufferSize, m_bufferSize);
        LADSPAControl::setBufferSize(m_monoBufferSize);
        
        if (m_pBufferLeft[0] != NULL)
        {
            delete [] m_pBufferLeft[0];
            delete [] m_pBufferLeft[1];
            delete [] m_pBufferRight[0];
            delete [] m_pBufferRight[1];
        }
        m_pBufferLeft[0] = new CSAMPLE[m_monoBufferSize];
        m_pBufferLeft[1] = new CSAMPLE[m_monoBufferSize];
        m_pBufferRight[0] = new CSAMPLE[m_monoBufferSize];
        m_pBufferRight[1] = new CSAMPLE[m_monoBufferSize];
    }

    if (m_pPot1 != NULL)
    {
        m_pControl1->setValue(m_pPot1->get());
        m_pControl2->setValue(m_pPot2->get());
        m_pControl3->setValue(m_pPot3->get());
        m_pControl4->setValue(m_pPot4->get());
    }

    for (int i = 0; i < m_monoBufferSize; i++)
    {
        m_pBufferLeft[0][i] = pIn[2 * i];
        m_pBufferRight[0][i] = pIn[2 * i + 1];
    }

    int bufferNo = 0;

    for (LADSPAInstanceList::iterator instance = m_Instances.begin(); instance != m_Instances.end(); instance++)
    {
        if ((*instance)->isInplaceBroken())
        {
            (*instance)->process(m_pBufferLeft[bufferNo], m_pBufferRight[bufferNo], m_pBufferLeft[1 - bufferNo], m_pBufferRight[1 - bufferNo], m_monoBufferSize);
            bufferNo = 1 - bufferNo;
        }
        else
        {
            (*instance)->process(m_pBufferLeft[bufferNo], m_pBufferRight[bufferNo], m_pBufferLeft[bufferNo], m_pBufferRight[bufferNo], m_monoBufferSize);
        }
    }

    CSAMPLE *pOutput = (CSAMPLE *)pOut;
    if (bufferNo == 0)
    {
        for (int i = 0; i < m_monoBufferSize; i++)
        {
            pOutput[2 * i] = m_pBufferLeft[0][i];
            pOutput[2 * i + 1] = m_pBufferRight[0][i];
        }
    }
    else
    {
        for (int i = 0; i < m_monoBufferSize; i++)
        {
            pOutput[2 * i] = m_pBufferLeft[1][i];
            pOutput[2 * i + 1] = m_pBufferRight[1][i];
        }
    }
}

void EngineLADSPA::addInstance(LADSPAInstance * instance)
{
    m_Instances.append(instance);
}
