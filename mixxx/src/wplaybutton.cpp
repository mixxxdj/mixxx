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

#include "images/playbutton_up.xpm"
#include "images/playbutton_down.xpm"

WPlayButton::WPlayButton(QWidget *parent, const char *name ) : QPushButton(parent,name)
{
    buttonUp   = new QPixmap(playbutton_up_xpm);
    buttonDown = new QPixmap(playbutton_down_xpm);
}

WPlayButton::~WPlayButton()
{
    delete buttonUp;
    delete buttonDown;
}

void WPlayButton::drawButton(QPainter *p)
{
    if (isDown())
        p->drawPixmap(0,0,*buttonDown);
    else
        p->drawPixmap(0,0,*buttonUp);
}

