/***************************************************************************
                          wselector.cpp  -  description
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

#include "wselector.h"

#include "images/selector-a.xpm"
#include "images/selector-b.xpm"

// Static member variable definition
QPixmap *WSelector::sliderA = 0;
QPixmap *WSelector::sliderB = 0;

WSelector::WSelector(QWidget *parent, const char *name ) : QSlider(parent,name)
{
    if (sliderA == 0)
    {
        sliderA = new QPixmap(slider_a_xpm);
        sliderB = new QPixmap(slider_b_xpm);
    }
    setMinValue(0);
    setMaxValue(1);
}

WSelector::~WSelector()
{
    delete sliderA;
    delete sliderB;
}

void WSelector::drawButton(QPainter *p)
{
    if (value==0)
        p->drawPixmap(0,0,*sliderA);
    else
        p->drawPixmap(0,0,*sliderB);
}

