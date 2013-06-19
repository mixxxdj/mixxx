/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENGINELADSPA_H
#define ENGINELADSPA_H

#include "engineobject.h"

#include "ladspa/ladspainstance.h"
#include "ladspa/ladspaplugin.h"

class ControlPotmeter;
class LADSPAControl;
class EngineLADSPA;

struct EngineLADSPAControlConnection
{
    ControlObject * potmeter;
    LADSPAControl * control;
    bool remove;
};

typedef QLinkedList<EngineLADSPAControlConnection *> EngineLADSPAControlConnectionLinkedList;
typedef QVector<EngineLADSPAControlConnection *> EngineLADSPAControlConnectionVector;

class EngineLADSPA : public EngineObject 
{
public:
    EngineLADSPA();
    ~EngineLADSPA();

    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
    void addInstance(LADSPAInstance * instance);
    EngineLADSPAControlConnection * addControl(ControlObject * potmeter, LADSPAControl * control);

    static EngineLADSPA * getEngine();

private:
    LADSPAInstanceLinkedList m_Instances;
    EngineLADSPAControlConnectionLinkedList m_Connections;
    int m_bufferSize;
    int m_monoBufferSize;
    CSAMPLE * m_pBufferLeft[2];
    CSAMPLE * m_pBufferRight[2];

    static EngineLADSPA * m_pEngine;
};

#endif
