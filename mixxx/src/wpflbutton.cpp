/***************************************************************************
                          wpflbutton.cpp  -  description
                             -------------------
    begin                : Sun Nov 17 2002
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

#include "wpflbutton.h"

#include "images/pfl-on.xpm"
#include "images/pfl-off.xpm"

// Static member variable definition
QPixmap *WPFLButton::buttonUp    = 0;
QPixmap *WPFLButton::buttonDown  = 0;

WPFLButton::WPFLButton(QWidget *parent, const char *name ) : QCheckBox(parent,name)
{
    if (buttonUp == 0)
    {
        buttonUp    = new QPixmap(pfl_off_xpm);
        buttonDown  = new QPixmap(pfl_on_xpm);
    }
    setBackgroundMode(NoBackground);
}

WPFLButton::~WPFLButton()
{
    delete buttonUp;
    delete buttonDown;
}

void WPFLButton::drawButton(QPainter *p)
{
    if (isOn())
        p->drawPixmap(0,0,*buttonDown);
    else
        p->drawPixmap(0,0,*buttonUp);
}




















