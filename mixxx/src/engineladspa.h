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

class EngineLADSPA : public EngineObject 
{
public:
    EngineLADSPA();
    ~EngineLADSPA();
    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);

private:
    LADSPAInstanceList m_Instances;
};

#endif
