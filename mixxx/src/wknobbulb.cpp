/***************************************************************************
                          wknobbulb.cpp  -  description
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

#include "wknobbulb.h"
#include <qpixmap.h>

#include "images/bulb_on.xpm"
#include "images/bulb_off.xpm"

WKnobBulb::WKnobBulb(QWidget *parent, const char *name ) : WKnob(parent,name)
{
    on = new QPixmap(bulb_on_xpm);
    off = new QPixmap(bulb_off_xpm);

    bulbState = false;
    setBackgroundMode(NoBackground);
}

WKnobBulb::~WKnobBulb()
{
    delete on, off;
}

void WKnobBulb::slotSetBulb(bool state)
{
    bulbState = state;
}

void WKnobBulb::repaintScreen(const QRect *cr)
{
    // Get knob pixmap
    QPixmap *k = getKnob();

    // Copy bulb into knob pixmap
 /*
    if (bulbState == false)
        k->loadFromData(bulb_off_xpm,
    else
 */

    // Paint picmap
    WKnob::repaintScreen(cr,k);
}

