/***************************************************************************
                          wpushbutton.cpp  -  description
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

#include "wpushbutton.h"

WPushButton::WPushButton(QWidget *parent, const char *name ) : QPushButton(parent,name)
{
    setToggleButton(true);

    connect(this,SIGNAL(toggled(bool)),this,SLOT(emitValueChanged(bool)));
}

WPushButton::~WPushButton()
{
}

void WPushButton::setValue(int v)
{
    if (v==0)
        setOn(false);
    else
        setOn(true);
}

void WPushButton::emitValueChanged(bool v)
{
    if (v)
    {
        emit(valueChanged(1));
        emit(valueOn());
    }
    else
        emit(valueChanged(0));
}

