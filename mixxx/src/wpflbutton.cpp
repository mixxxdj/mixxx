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
int WPFLButton::instantiateNo = 0;

WPFLButton::WPFLButton(QWidget *parent, const char *name ) : QCheckBox(parent,name)
{
    instantiateNo++;
    if (instantiateNo==1)
    {
        buttonUp    = new QPixmap(pfl_off_xpm);
        buttonDown  = new QPixmap(pfl_on_xpm);
    }
    setToggleButton(true);

    setBackgroundMode(NoBackground);

    connect(this,SIGNAL(toggled(bool)),this,SLOT(emitValueChanged(bool)));
}

WPFLButton::~WPFLButton()
{
    instantiateNo--;
    if (instantiateNo==0)
    {
        delete buttonUp;
        delete buttonDown;
    }
}

void WPFLButton::setValue(int v)
{
    if (v==0)
        setChecked(false);
    else
        setChecked(true);
}

void WPFLButton::emitValueChanged(bool v)
{
    if (v)
        emit(valueChanged(1));
    else
        emit(valueChanged(0));
}

void WPFLButton::drawButton(QPainter *p)
{
    if (isOn())
        p->drawPixmap(0,0,*buttonDown);
    else
        p->drawPixmap(0,0,*buttonUp);
}




















