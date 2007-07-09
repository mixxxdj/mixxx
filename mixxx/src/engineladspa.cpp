/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "engineladspa.h"


EngineLADSPA::EngineLADSPA()
{
}

EngineLADSPA::~EngineLADSPA()
{
}

void EngineLADSPA::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize)
{
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
