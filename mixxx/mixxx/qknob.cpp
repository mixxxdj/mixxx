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
#include "knob045.xpm"
#include "knob090.xpm"
#include "knob135.xpm"
#include "knob180.xpm"
#include "knob225.xpm"
#include "knob270.xpm"
#include "knob315.xpm"

QKnob::QKnob(QWidget *parent, const char *name ) : QDial(parent,name)
{
	button000 = new QPixmap(knob000_xpm);
	button045 = new QPixmap(knob045_xpm);
	button090 = new QPixmap(knob090_xpm);
	button135 = new QPixmap(knob135_xpm);
	button180 = new QPixmap(knob180_xpm);
	button225 = new QPixmap(knob225_xpm);
	button270 = new QPixmap(knob270_xpm);
	button315 = new QPixmap(knob315_xpm);

	repaintRect = 0;
}

QKnob::~QKnob()
{
	delete button315;
	delete button270;
	delete button225;
	delete button180;
	delete button135;
	delete button090;
	delete button045;
	delete button000;
}

void QKnob::repaintScreen(const QRect *cr=0)
{
	if (cr != 0)
	{
		if (repaintRect != 0)
			delete repaintRect;
		repaintRect = new QRect(cr->topLeft(),cr->bottomRight());
    }

	QRect src(repaintRect->topLeft(),repaintRect->bottomRight());
	int range = (maxValue()-minValue());
	int val = value()*8/range;

	qDebug("Value: %i (%p)",val,cr);

	QPixmap *b = button000;
	switch (val)
	{
		case 4: b = button000; break;
		case 5: b = button045; break;
		case 6: b = button090; break;
		case 7: b = button135; break;
		case 8: b = button180; break;
		case 0: b = button180; break;
		case 1: b = button225; break;
		case 2: b = button270; break;
		case 3: b = button315; break;
	}

	bitBlt(this,repaintRect->topLeft(),b,src);
}