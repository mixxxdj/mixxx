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

// Static member variable definition
QPixmap *WPlayposSlider::slider = 0;
QPixmap *WPlayposSlider::marker = 0;

WPlayposSlider::WPlayposSlider(QWidget *parent, const char *name ) : QWidget(parent,name)
{
    if (slider==0)
    {
        slider = new QPixmap(playposslider_xpm);
        marker = new QPixmap(playposmarker_xpm);
    }

    value = 0;
    poss = 2;
    pix_length = 192; // Length of slider in pixels: len (202) - border(4) - marker len (6)

    //setBackgroundMode(NoBackground);
}

WPlayposSlider::~WPlayposSlider()
{
    delete slider;
    delete marker;
}

void WPlayposSlider::mouseMoveEvent(QMouseEvent *e)
{
    poss = e->x()-3; // X pos - half marker len
    if (poss>194)
        poss = 194;
    else if (poss<2)
        poss = 2;

    // Update display
    update();
}


void WPlayposSlider::mousePressEvent(QMouseEvent *e)
{
    mouseMoveEvent(e);
}

void WPlayposSlider::mouseReleaseEvent(QMouseEvent *e)
{
    mouseMoveEvent(e);

    // Calculate and emit new value
    int value = (int)((CSAMPLE)(poss-2)*127./(CSAMPLE)pix_length);
    emit(valueChanged(value));
}

void WPlayposSlider::setValue(int v)
{
    // Set value without emitting a valueChanged signal, and update display
    value = v;
    poss = (int)((CSAMPLE)value*((CSAMPLE)pix_length/127.))+2;

    repaint();
}

void WPlayposSlider::paintEvent(QPaintEvent *)
{
    bitBlt(this, 0, 0, slider);
    bitBlt(this, poss, 2, marker);
    
    //paint.drawPixmap(0,0,*slider);
    //paint.drawPixmap(poss,2,*marker);
}
