/***************************************************************************
                          dlgplaycontrol.cpp  -  description
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

#include "dlgplaycontrol.h"
#include "wplaybutton.h"
#include <qgroupbox.h>

DlgPlaycontrol::DlgPlaycontrol(QWidget *parent, const char *name ) : DlgPlaycontrolDlg(parent,name)
{
}

DlgPlaycontrol::~DlgPlaycontrol()
{
}

void DlgPlaycontrol::layoutMirror()
{
    // Mirror volume and knobs positions
    QPoint playpos = PushButtonPlay->pos();
    playpos.setX(playpos.x()+34); // Diff in width between playbutton and rate control
    PushButtonPlay->move(GroupBoxRate->pos());
    GroupBoxRate->move(playpos);
}
