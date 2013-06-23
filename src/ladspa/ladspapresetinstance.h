/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LADSPAPRESETINSTANCE_H
#define LADSPAPRESETINSTANCE_H

#include "ladspaplugin.h"
#include "ladspainstance.h"
#include "engine/engineladspa.h"

class ControlPotmeter;
class LADSPAPresetKnob;

typedef QVector<ConfigKey *> ConfigKeyVector;

class LADSPAPresetInstance
{
public:
    LADSPAPresetInstance(int pluginCount, int controlCount, int slot);
    ~LADSPAPresetInstance();

    void addPlugin(int i, LADSPAPlugin * plugin, EngineLADSPA * engine);
    void addControl(int i, LADSPAPresetKnob * knob, EngineLADSPA * engine);
    int getKnobCount();
    ConfigKey getKey(int i);

private:
    int m_iInstanceID;
    static int m_iNextInstanceID;
    LADSPAInstanceVector m_Instances;
    EngineLADSPAControlConnectionVector m_Connections;
    ConfigKeyVector m_Keys;
    int m_iSlotNumber;
};

#endif
