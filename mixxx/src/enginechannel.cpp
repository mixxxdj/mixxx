/***************************************************************************
                          enginechannel.cpp  -  description
                             -------------------
    begin                : Sun Apr 28 2002
    copyright            : (C) 2002 by 
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

#include "enginechannel.h"
#include "wslider.h"
#include "controlengine.h"
#include "controlpushbutton.h"
#include "engineclipping.h"
#include "enginepregain.h"
#include "enginevolume.h"
#include "enginefilterblock.h"
#include "engineflanger.h"
#include "dlgchannel.h"

EngineChannel::EngineChannel(DlgChannel *dlg, const char *group)
{
    // Pregain:
    pregain = new EnginePregain(dlg->DialGain, group);

    // Filters:
    filter = new EngineFilterBlock(dlg->DialFilterLow,
                                   dlg->DialFilterMiddle,
                                   dlg->DialFilterHigh, group);

    // Clipping:
    clipping = new EngineClipping(dlg->BulbClipping);

    // Volume control:
    volume = new EngineVolume(dlg->SliderVolume,ConfigKey(group,"volume"));

    // PFL button
    ControlPushButton *p = new ControlPushButton(ConfigKey(group, "pfl" ));
    p->setWidget(dlg->CheckBoxPFL);
    pfl = new ControlEngine(p);
}

EngineChannel::~EngineChannel()
{
    delete pregain;
    delete filter;
    delete clipping;
    delete volume;
}

ControlEngine *EngineChannel::getPFL()
{
    return pfl;
}

CSAMPLE *EngineChannel::process(const CSAMPLE* source, const int buffer_size)
{
    CSAMPLE *temp  = pregain->process(source, buffer_size);
    CSAMPLE *temp2 = clipping->process(temp, buffer_size);
    temp = filter->process(temp2, buffer_size); 
    temp2 = volume->process(temp, buffer_size);

    return temp2;
}





