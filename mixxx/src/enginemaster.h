/***************************************************************************
                          enginemaster.h  -  description
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

#ifndef ENGINEMASTER_H
#define ENGINEMASTER_H

#include "engineobject.h"
#include "enginebuffer.h"
#include "dlgmaster.h"
#include "dlgcrossfader.h"
#include "enginevolume.h"
#include "enginechannel.h"
#include "engineclipping.h"
#include "controlpotmeter.h"

/**
  *@author 
  */

class EngineMaster : public EngineObject  {
  Q_OBJECT
private:
    EngineBuffer *buffer1, *buffer2;
    EngineChannel *channel1, *channel2;
    EngineVolume *volume, *head_volume;
    ControlPotmeter *crossfader, *head_mix;
    EngineClipping *clipping, *head_clipping;
    CSAMPLE *out, *tmp, *tmp2, *tmp3;
    bool left, right, head1, head2;
public:
    EngineMaster(DlgMaster *master_dlg, DlgCrossfader *crossfader_dlg,
                 DlgChannel *channel1_dlg, DlgChannel *channel2_dlg,
                 EngineBuffer *buffer1, EngineBuffer *buffer2,
                 EngineChannel *, EngineChannel *,
                 const char *group);
    ~EngineMaster();
    CSAMPLE *process(const CSAMPLE *, const int);
public slots:
    void slotChannelLeft(bool toggle);
    void slotChannelRight(bool toggle);
    void slotChannelHead1(int toggle);
    void slotChannelHead2(int toggle);
};

#endif
