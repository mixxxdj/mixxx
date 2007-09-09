/***************************************************************************
                          powermate.cpp  -  description
                             -------------------
    begin                : Tue Apr 29 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qapplication.h"
#include "powermate.h"
#include "rotary.h"
#include "controlobject.h"
#include "controleventmidi.h"
#include "midiobject.h"
#include "mathstuff.h"

PowerMate::PowerMate() : Input(), m_qRequestLed(5)
{
    m_pRotary = new Rotary();
    m_pControlObjectButton = 0;
    m_pControlObjectRotary = 0;
}

PowerMate::~PowerMate()
{
    if (running())
    {
        terminate();
        wait();
    }
    delete m_pRotary;
}

QStringList PowerMate::getMappings()
{
    QStringList mappings;
    mappings << kqInputMappingPositionP1 << kqInputMappingPositionP2 << kqInputMappingSongP1 << kqInputMappingSongP2;
    return mappings;
}

void PowerMate::selectMapping(QString mapping)
{
    if (mapping==kqInputMappingPositionP1)
    {
        m_pControlObjectRotary = ControlObject::getControl(ConfigKey("[Channel1]","scratch"));
        m_pControlObjectButton = ControlObject::getControl(ConfigKey("[Channel1]","play"));
        m_ctrlVuMeter = ControlObject::getControl(ConfigKey("[Channel1]","VuMeter"));
        m_pRotary->setFilterLength(kiRotaryFilterMaxLen);
    }
    else if (mapping==kqInputMappingPositionP2)
    {
        m_pControlObjectRotary = ControlObject::getControl(ConfigKey("[Channel2]","scratch"));
        m_pControlObjectButton = ControlObject::getControl(ConfigKey("[Channel2]","play"));
        m_ctrlVuMeter = ControlObject::getControl(ConfigKey("[Channel2]","VuMeter"));
        m_pRotary->setFilterLength(kiRotaryFilterMaxLen);
    }
    else if (mapping==kqInputMappingSongP1)
    {
        m_pControlObjectRotary = ControlObject::getControl(ConfigKey("[Channel1]","rateSearch"));
        m_pControlObjectButton = ControlObject::getControl(ConfigKey("[Channel1]","play"));
        m_ctrlVuMeter = ControlObject::getControl(ConfigKey("[Channel1]","VuMeter"));
        m_pRotary->setFilterLength(kiRotaryFilterMaxLen);
    }
    else if (mapping==kqInputMappingSongP2)
    {
        m_pControlObjectRotary = ControlObject::getControl(ConfigKey("[Channel2]","realsearch"));
        m_pControlObjectButton = ControlObject::getControl(ConfigKey("[Channel2]","play"));
        m_ctrlVuMeter = ControlObject::getControl(ConfigKey("[Channel2]","VuMeter"));
        m_pRotary->setFilterLength(kiRotaryFilterMaxLen);
    }
}

void PowerMate::led()
{
    m_qRequestLed.tryAccess(1);
}
