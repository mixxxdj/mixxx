/***************************************************************************
                          enginevinylcontrol.h  -  description
                             -------------------
    copyright            : (C) 2007 by Albert Santoni
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENGINEVINYLCONTROL_H
#define ENGINEVINYLCONTROL_H

#include "vinylcontrolproxy.h"
#include "engineobject.h"

class EngineVinylControl : public EngineObject
{
public:
    EngineVinylControl(ConfigObject<ConfigValue> *pConfig, const char *group);
    ~EngineVinylControl();
    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
private:
    VinylControlProxy* m_pVinylControl;
    ConfigObject<ConfigValue> *m_pConfig;
};

#endif
