//
// C++ Implementation: mouse
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "mouse.h"
#include "rotary.h"
#include <qstringlist.h>
#include "controlobject.h"

#ifdef __LINUX__
#include "mouselinux.h"
#endif

QPtrList<Mouse> Mouse::m_sqInstanceList;

Mouse::Mouse() : Input()
{
    m_pRotary = new Rotary();
    m_sqInstanceList.append(this);
}

Mouse::~Mouse()
{
    delete m_pRotary;
    m_sqInstanceList.remove(this);
}

QStringList Mouse::getMappings()
{
    QStringList mappings;
    mappings << kqInputMappingPositionP1 << kqInputMappingPositionP2 << kqInputMappingSongP1 << kqInputMappingSongP2;
    return mappings;
}

void Mouse::selectMapping(QString mapping)
{
    if (mapping==kqInputMappingPositionP1)
    {
        m_pControlObjectRotary = ControlObject::getControl(ConfigKey("[Channel1]","wheel"));
        m_pRotary->setFilterLength(kiRotaryFilterMaxLen);
    }
    else if (mapping==kqInputMappingPositionP2)
    {
        m_pControlObjectRotary = ControlObject::getControl(ConfigKey("[Channel2]","wheel"));
        m_pRotary->setFilterLength(kiRotaryFilterMaxLen);
    }
    else if (mapping==kqInputMappingSongP1)
    {
        m_pControlObjectRotary = ControlObject::getControl(ConfigKey("[Channel1]","rateSearch"));
        m_pRotary->setFilterLength(kiRotaryFilterMaxLen);
    }
    else if (mapping==kqInputMappingSongP2)
    {
        m_pControlObjectRotary = ControlObject::getControl(ConfigKey("[Channel2]","realsearch"));
        m_pRotary->setFilterLength(kiRotaryFilterMaxLen);
    }
}

void Mouse::destroyAll()
{
    for (Mouse *p = m_sqInstanceList.first(); p; p = m_sqInstanceList.first())
        delete p;
}


QStringList Mouse::getDeviceList()
{
#ifdef __LINUX__
    return MouseLinux::getDeviceList();
#endif
#ifndef __LINUX__
    return QStringList("None");
#endif
}

void Mouse::calibrateStart()
{
    m_pRotary->calibrateStart();
}

double Mouse::calibrateEnd()
{
    return m_pRotary->calibrateEnd();
}

void Mouse::setCalibration(double c)
{
    m_pRotary->setCalibration(c);
}

