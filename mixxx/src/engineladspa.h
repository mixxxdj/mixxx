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

#include "ladspainstance.h"
#include "ladspaplugin.h"

#include <Qt3Support>

class ControlPotmeter;
class LADSPAControl;
class EngineLADSPA;

struct EngineLADSPAControlConnection
{
    ControlPotmeter * potmeter;
    LADSPAControl * control;
    bool remove;
};

typedef Q3PtrList<EngineLADSPAControlConnection> EngineLADSPAControlConnectionList;
typedef Q3PtrVector<EngineLADSPAControlConnection> EngineLADSPAControlConnectionVector;

class EngineLADSPA : public EngineObject 
{
public:
    EngineLADSPA();
    ~EngineLADSPA();

    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
    void addInstance(LADSPAInstance * instance);
    EngineLADSPAControlConnection * addControl(ControlPotmeter * potmeter, LADSPAControl * control);

    static EngineLADSPA * getEngine();

private:
    LADSPAInstanceList m_Instances;
    EngineLADSPAControlConnectionList m_Connections;
    int m_bufferSize;
    int m_monoBufferSize;
    CSAMPLE * m_pBufferLeft[2];
    CSAMPLE * m_pBufferRight[2];

    static EngineLADSPA * m_pEngine;
};

#endif
