/***************************************************************************
                          wwheel.cpp  -  description
                             -------------------
    begin                : Mon Jun 24 2002
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

#include "wwheel.h"
#include <qpainter.h>

#include "images/wheel.cpp"

// Static member variable definition
QPixmap **WWheel::pix = 0;

WWheel::WWheel(QWidget *parent, const char *name ) : QWidget(parent,name)
{
    oldvalue = 49;
    value = 49;

    // Convert xpm's to pixmaps
    if (pix == 0)
    {
        pix = new QPixmap*[100];
        for (int i=0; i<100; i++)
            pix[i] = new QPixmap(wheel[i]);
    }

    setBackgroundMode(NoBackground);
}

WWheel::~WWheel()
{
   for (int i=0; i<100; i++)
       delete pix[i];
   delete [] pix;
}

void WWheel::mouseMoveEvent(QMouseEvent *e)
{
    oldvalue = value;
    value = (e->x()-startval+49);
    if (value>99)
        value = 99;
    else if (value<0)
        value = 0;

    emit(valueChanged(value));
    update();
}

void WWheel::mousePressEvent(QMouseEvent *e)
{
    startval = e->x();
}

void WWheel::mouseReleaseEvent(QMouseEvent *)
{
    oldvalue = value;
    value = 49;
    emit(valueChanged(value));
    update();
}

void WWheel::paintEvent(QPaintEvent *)
{
    bitBlt(this, 0, 0, pix[value]);
}

void WWheel::setValue(int v)
{
    value = v;
    update();
}
