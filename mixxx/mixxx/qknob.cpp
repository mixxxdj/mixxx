/***************************************************************************
                          qknob.cpp  -  description
                             -------------------
    begin                : Wed Feb 27 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qknob.h"
#include <qrect.h>

#include "knob000.xpm"
#include "knob090.xpm"
#include "knob180.xpm"
#include "knob270.xpm"

QKnob::QKnob(QWidget *parent, const char *name ) : QDial(parent,name)
{
	button000 = new QPixmap(knob000_xpm);
	button090 = new QPixmap(knob090_xpm);
	button180 = new QPixmap(knob180_xpm);
	button270 = new QPixmap(knob270_xpm);

	repaintRect = 0;
}

QKnob::~QKnob()
{
	delete button270;
	delete button180;
	delete button090;
	delete button000;
}

void QKnob::repaintScreen(const QRect *cr=0)
{
	qDebug("Value: %i (%p)",value(),cr);
	if (cr != 0)
	{
		if (repaintRect != 0)
			delete repaintRect;
		repaintRect = new QRect(cr->topLeft(),cr->bottomRight());
    }

	QRect src(repaintRect->topLeft(),repaintRect->bottomRight());
//	int c = (maxValue()-minValue())/4;

	QPixmap *b = 0;
	if (value() > 50)
		b = button000;
	else
		b = button090;
	bitBlt(this,repaintRect->topLeft(),b,src);
}