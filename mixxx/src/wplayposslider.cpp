/***************************************************************************
                          wplayposslider.cpp  -  description
                             -------------------
    begin                : Tue Jun 25 2002
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

#include "wplayposslider.h"
#include <qpainter.h>
#include "defs.h"

#include "images/playposslider.xpm"
#include "images/playposmarker.xpm"

WPlayposSlider::WPlayposSlider(QWidget *parent, const char *name ) : QWidget(parent,name)
{
    slider = new QPixmap(playposslider_xpm);
    marker = new QPixmap(playposmarker_xpm);

    value = 0;
    pix_length = 194; // Length of slider in pixels
}

WPlayposSlider::~WPlayposSlider()
{
    delete slider;
    delete marker;
}

void WPlayposSlider::mouseMoveEvent(QMouseEvent *e)
{
    int x = e->x();
    if (x>199)
        x = 199;
    else if (x<=0)
        x = 0;

    // x is mouse position, in range from 0 to 199
    int value = (int)((CSAMPLE)x*100./200.);
    //qDebug("x: %i, value: %i",x,value);

    // Emit valueChanged signal
    emit(valueChanged(value));

    // Update display
    update();
}


void WPlayposSlider::mousePressEvent(QMouseEvent *e)
{
    mouseMoveEvent(e);
}

void WPlayposSlider::setValue(int v)
{
    // Set value without emitting a valueChanged signal, and force display update
    value = v;
    update();
}

void WPlayposSlider::paintEvent(QPaintEvent *e)
{
    QPainter paint(this);

    int pos = (int)((CSAMPLE)value*((CSAMPLE)pix_length/100.))+2;

    paint.drawPixmap(0,0,*slider);
    paint.drawPixmap(pos,2,*marker);
}
