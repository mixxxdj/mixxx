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
#include "ladspaloader.h"
#include "ladspaplugin.h"

class ControlPotmeter;
class LADSPAControl;

class EngineLADSPA : public EngineObject 
{
public:
    EngineLADSPA();
    ~EngineLADSPA();

    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
    void addInstance(LADSPAInstance * instance);

private:
    LADSPAInstanceList m_Instances;
    LADSPALoader * m_Loader;
    int m_bufferSize;
    ControlPotmeter * m_pPot1;
    ControlPotmeter * m_pPot2;
    ControlPotmeter * m_pPot3;
    ControlPotmeter * m_pPot4;
    LADSPAControl * m_pControl1;
    LADSPAControl * m_pControl2;
    LADSPAControl * m_pControl3;
    LADSPAControl * m_pControl4;
};

#endif
