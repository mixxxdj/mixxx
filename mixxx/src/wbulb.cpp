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

// Static member variable definition
QPixmap *WBulb::bulbOn = 0;
QPixmap *WBulb::bulbOff = 0;
int WBulb::instantiateNo = 0;

WBulb::WBulb(QWidget *parent, const char *name ) : QRadioButton(parent,name)
{
    instantiateNo++;
    if (instantiateNo==1)
    {
        bulbOn  = new QPixmap(bulb_on_xpm);
        bulbOff = new QPixmap(bulb_off_xpm);
    }
    setBackgroundMode(NoBackground);

    connect(this,SIGNAL(toggled(bool)),this,SLOT(emitValueChanged(bool)));
}

WBulb::~WBulb()
{
    instantiateNo--;
    if (instantiateNo==0)
    {
        delete bulbOn;
        delete bulbOff;
    }
}

void WBulb::drawButton(QPainter *p)
{
    if (isOn())
        p->drawPixmap(0,0,*bulbOn);
    else
        p->drawPixmap(0,0,*bulbOff);
}

void WBulb::mousePressEvent(QMouseEvent *)
{
}

void WBulb::setValue(int v)
{
    if (v==0)
        setOn(false);
    else
        setOn(true);
}

void WBulb::emitValueChanged(bool v)
{
    if (v)
    {
        emit(valueChanged(1));
        emit(valueOn());
    }
    else
        emit(valueChanged(0));
}

