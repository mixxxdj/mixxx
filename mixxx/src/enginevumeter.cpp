/***************************************************************************
                          enginevumeter.cpp  -  description
                             -------------------
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
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

#include "enginevumeter.h"
#include "dlgvumeter.h"
#include "controlengine.h"
#include "controlvumeter.h"

EngineVUmeter::EngineVUmeter(DlgVUmeter *dlgVUmeter, const char *group)
{
    ControlVUmeter *ctrlVUmeter = new ControlVUmeter(ConfigKey(group, "VUmeter"), dlgVUmeter);
    ctrlVUmeter->setWidget(dlgVUmeter);
    leds = new ControlEngine(ctrlVUmeter);
    leds->set(0.);
}

EngineVUmeter::~EngineVUmeter()
{
    delete leds;
}

CSAMPLE *EngineVUmeter::process(const CSAMPLE *source, const int buffer_size)
{
    // Calculate the rms volume
    FLOAT_TYPE fRMSvolume = 0;
    for (int i=0; i<buffer_size; i++)
    {
        fRMSvolume += source[i]*source[i];
    }
    fRMSvolume = sqrt(fRMSvolume/buffer_size);

    leds->set(fRMSvolume);

    return 0;
}
