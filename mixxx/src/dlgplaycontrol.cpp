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
#include "wplayposslider.h"
#include "wwheel.h"
#include <qgroupbox.h>
#include <qlabel.h>

DlgPlaycontrol::DlgPlaycontrol(QWidget *parent, const char *name ) : DlgPlaycontrolDlg(parent,name)
{
}

DlgPlaycontrol::~DlgPlaycontrol()
{
}

void DlgPlaycontrol::layoutMirror()
{
    // Move rate slider
    QPoint pos = GroupBoxRate->pos();
    GroupBoxRate->move(208,pos.y());

    // Move playbutton
    pos = PushButtonPlay->pos();
    PushButtonPlay->move(5,pos.y());

    // Move playpos slider, display and wheel
    pos = SliderPosition->pos();
    SliderPosition->move(5,pos.y());
    pos = textLabelTrack->pos();
    textLabelTrack->move(5,pos.y());
    pos = WheelPlaycontrol->pos();
    WheelPlaycontrol->move(5,pos.y());

    // Move heading and relabel
    TextHeading->move(5,TextHeading->pos().y());
    TextHeading->setText(QString("PLAYER B"));
}
