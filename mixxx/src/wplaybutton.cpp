/***************************************************************************
                          wplaybutton.cpp  -  description
                             -------------------
    begin                : Fri Jun 21 2002
    copyright            : (C) 2002 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "wplaybutton.h"

#include "images/playbutton_up_on.xpm"
#include "images/playbutton_up_off.xpm"
#include "images/playbutton_down_on.xpm"
#include "images/playbutton_down_off.xpm"

// Static member variable definition
QPixmap *WPlayButton::buttonUpOn    = 0;
QPixmap *WPlayButton::buttonUpOff   = 0;
QPixmap *WPlayButton::buttonDownOn  = 0;
QPixmap *WPlayButton::buttonDownOff = 0;

WPlayButton::WPlayButton(QWidget *parent, const char *name ) : QPushButton(parent,name)
{
    if (buttonUpOn == 0)
    {
        buttonUpOn    = new QPixmap(playbutton_up_on_xpm);
        buttonUpOff   = new QPixmap(playbutton_up_off_xpm);
        buttonDownOn  = new QPixmap(playbutton_down_on_xpm);
        buttonDownOff = new QPixmap(playbutton_down_off_xpm);
    }
    setToggleButton(true);

    setBackgroundMode(NoBackground);

    connect(this,SIGNAL(toggled(bool)),this,SLOT(emitValueChanged(bool)));
}

WPlayButton::~WPlayButton()
{
    delete buttonUpOn;
    delete buttonUpOff;
    delete buttonDownOn;
    delete buttonDownOff;
}

void WPlayButton::setValue(int v)
{
    if (v==0)
        setOn(false);
    else
        setOn(true);
}

void WPlayButton::emitValueChanged(bool v)
{
    if (v)
        emit(valueChanged(1));
    else
        emit(valueChanged(0));
}

void WPlayButton::drawButton(QPainter *p)
{
    if (isDown())
    {
        if (isOn())
            p->drawPixmap(0,0,*buttonDownOn);
        else
            p->drawPixmap(0,0,*buttonDownOff);
    }
    else
    {
        if (isOn())
            p->drawPixmap(0,0,*buttonUpOn);
        else
            p->drawPixmap(0,0,*buttonUpOff);
    }
}
