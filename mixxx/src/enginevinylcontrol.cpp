/***************************************************************************
                          enginevinylcontrol.cpp  -  description
                             -------------------
    copyright            : (C) 2007 by Albert Santoni
    email                : gamegod \a\t users.sf.net
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtDebug>
#include "enginevinylcontrol.h"
#include "controlpotmeter.h"

/*----------------------------------------------------------------
   NOTE: This class is currently a WIP. It _MIGHT_ one day hold the vinyl control
   interface.

   ----------------------------------------------------------------*/

EngineVinylControl::EngineVinylControl(ConfigObject<ConfigValue> * pConfig, const char * group)
{
    m_pConfig = pConfig;
    m_pVinylControl = new VinylControlProxy(pConfig, group);
}

EngineVinylControl::~EngineVinylControl()
{
    delete m_pVinylControl;
}

void EngineVinylControl::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{
    CSAMPLE * pOutput = (CSAMPLE *)pOut;

    qCritical() << "FIXME: EngineVinylControl::process isn't done";
    //The vinyl control objects want short integer arrays, not CSAMPLE (float) arrays... eeek
    //m_pVinylControl->AnalyseSamples(pIn, iBufferSize);

    /*
       for (int i=0; i<iBufferSize; ++i)
       {

       }
     */
}

