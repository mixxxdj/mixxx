/***************************************************************************
                          dlgchannel.cpp  -  description
                             -------------------
    begin                : Wed Feb 20 2002
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

#include "dlgchannel.h"
#include <qgroupbox.h>

DlgChannel::DlgChannel(QWidget *parent, const char *name ) : DlgChannelDlg(parent,name)
{
}

DlgChannel::~DlgChannel()
{
}

void DlgChannel::layoutMirror()
{
    // Mirror volume and knobs positions
    // QPoint knobpos = GroupBoxKnobs->pos();
    //GroupBoxKnobs->move(GroupBoxVolume->pos());
    //GroupBoxVolume->move(knobpos);
}
