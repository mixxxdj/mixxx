/***************************************************************************
                          wbulb.cpp  -  description
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

#include "wbulb.h"

#include <qpixmap.h>

#include "images/bulb_on.xpm"
#include "images/bulb_off.xpm"

WBulb::WBulb(QWidget *parent, const char *name ) : QRadioButton(parent,name)
{
    bulbOn  = new QPixmap(bulb_on_xpm);
    bulbOff = new QPixmap(bulb_off_xpm);
}

WBulb::~WBulb()
{
    delete bulbOn;
    delete bulbOff;
}

void WBulb::drawButton(QPainter *p)
{
    if (isOn())
        p->drawPixmap(0,0,*bulbOn);
    else
        p->drawPixmap(0,0,*bulbOff);
}
