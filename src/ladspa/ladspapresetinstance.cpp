/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtCore>
#include "engine/engineladspa.h"
#include "ladspapresetinstance.h"
#include "ladspapresetknob.h"
#include "controlpotmeter.h"
#include "ladspacontrol.h"
#include "configobject.h"

int LADSPAPresetInstance::m_iNextInstanceID = 0;

LADSPAPresetInstance::LADSPAPresetInstance(int pluginCount, int controlCount, int slot)
{
    m_iInstanceID = m_iNextInstanceID;
    m_iNextInstanceID++;

    m_Instances.resize(pluginCount);
    m_Connections.resize(controlCount);
    m_Keys.resize(controlCount);

    m_iSlotNumber = slot;
}

LADSPAPresetInstance::~LADSPAPresetInstance()
{
    for (int i = 0; i < m_Instances.count(); i++)
    {
        m_Instances[i]->remove = 1;
    }
    for (int i = 0; i < m_Connections.count(); i++)
    {
        m_Connections[i]->remove = 1;
    }
}

void LADSPAPresetInstance::addPlugin(int i, LADSPAPlugin * plugin, EngineLADSPA * engine)
{
    LADSPAInstance * instance = plugin->instantiate(m_iSlotNumber);
    m_Instances [i] = instance;
    engine->addInstance(instance);
}

void LADSPAPresetInstance::addControl(int i, LADSPAPresetKnob * knob, EngineLADSPA * engine)
{
    ConfigKey * key = new ConfigKey("[LADSPA]", knob->getLabel());
    ControlObject * potmeter = new ControlPotmeter(*key, knob->getMin(), knob->getMax());
    ControlObjectThreadMain * potmeterThreadMain = new ControlObjectThreadMain(potmeter);
    LADSPAControl * control = new LADSPAControl;
    control->setValue(knob->getDefault());
    potmeterThreadMain->slotSet(knob->getDefault());
    m_Keys[i] = key;
    LADSPAPortConnectionVector * portConnections = knob->getConnections();
    for (LADSPAPortConnectionVector::iterator connection = portConnections->begin(); connection != portConnections->end(); connection++)
    {
        m_Instances[(*connection).plugin]->connect((*connection).port, control->getBuffer());
    }
    m_Connections[i] = engine->addControl(potmeter, control);
}

int LADSPAPresetInstance::getKnobCount()
{
    return m_Connections.count();
}

ConfigKey LADSPAPresetInstance::getKey(int i)
{
    return *(m_Keys[i]);
}
