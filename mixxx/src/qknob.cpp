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

#include "qknob/knob0.xpm"
#include "qknob/knob1.xpm"
#include "qknob/knob2.xpm"
#include "qknob/knob3.xpm"
#include "qknob/knob4.xpm"
#include "qknob/knob5.xpm"
#include "qknob/knob6.xpm"
#include "qknob/knob7.xpm"
#include "qknob/knob8.xpm"
#include "qknob/knob9.xpm"
#include "qknob/knob10.xpm"
#include "qknob/knob11.xpm"
#include "qknob/knob12.xpm"
#include "qknob/knob13.xpm"
#include "qknob/knob14.xpm"
#include "qknob/knob15.xpm"
#include "qknob/knob16.xpm"
#include "qknob/knob17.xpm"
#include "qknob/knob18.xpm"
#include "qknob/knob19.xpm"
#include "qknob/knob20.xpm"
#include "qknob/knob21.xpm"
#include "qknob/knob22.xpm"
#include "qknob/knob23.xpm"
#include "qknob/knob24.xpm"
#include "qknob/knob25.xpm"
#include "qknob/knob26.xpm"
#include "qknob/knob27.xpm"
#include "qknob/knob28.xpm"
#include "qknob/knob29.xpm"
#include "qknob/knob30.xpm"
#include "qknob/knob31.xpm"

QKnob::QKnob(QWidget *parent, const char *name ) : QDial(parent,name)
{
	button00 = new QPixmap(knob0_xpm);
	button01 = new QPixmap(knob1_xpm);
	button02 = new QPixmap(knob2_xpm);
	button03 = new QPixmap(knob3_xpm);
	button04 = new QPixmap(knob4_xpm);
	button05 = new QPixmap(knob5_xpm);
	button06 = new QPixmap(knob6_xpm);
	button07 = new QPixmap(knob7_xpm);
	button08 = new QPixmap(knob8_xpm);
	button09 = new QPixmap(knob9_xpm);
	button10 = new QPixmap(knob10_xpm);
	button11 = new QPixmap(knob11_xpm);
	button12 = new QPixmap(knob12_xpm);
	button13 = new QPixmap(knob13_xpm);
	button14 = new QPixmap(knob14_xpm);
	button15 = new QPixmap(knob15_xpm);
	button16 = new QPixmap(knob16_xpm);
	button17 = new QPixmap(knob17_xpm);
	button18 = new QPixmap(knob18_xpm);
	button19 = new QPixmap(knob19_xpm);
	button20 = new QPixmap(knob20_xpm);
	button21 = new QPixmap(knob21_xpm);
	button22 = new QPixmap(knob22_xpm);
	button23 = new QPixmap(knob23_xpm);
	button24 = new QPixmap(knob24_xpm);
	button25 = new QPixmap(knob25_xpm);
	button26 = new QPixmap(knob26_xpm);
	button27 = new QPixmap(knob27_xpm);
	button28 = new QPixmap(knob28_xpm);
	button29 = new QPixmap(knob29_xpm);
	button30 = new QPixmap(knob30_xpm);
	button31 = new QPixmap(knob31_xpm);

	repaintRect = 0;
}

QKnob::~QKnob()
{
	delete button00;
	delete button01;
	delete button02;
	delete button03;
	delete button04;
	delete button05;
	delete button06;
	delete button07;
	delete button08;
	delete button09;
	delete button10;
	delete button11;
	delete button12;
	delete button13;
	delete button14;
	delete button15;
	delete button16;
	delete button17;
	delete button18;
	delete button19;
	delete button20;
	delete button21;
	delete button22;
	delete button23;
	delete button24;
	delete button25;
	delete button26;
	delete button27;
	delete button28;
	delete button29;
	delete button30;
	delete button31;
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
	int val = value()*32/range;

	//qDebug("Value: %i (%p)",val,cr);

	QPixmap *b = button00;
	switch (val)
	{
		case 15: b = button00; break;
		case 16: b = button01; break;
		case 17: b = button02; break;
		case 18: b = button03; break;
		case 19: b = button04; break;
		case 20: b = button05; break;
		case 21: b = button06; break;
		case 22: b = button07; break;
		case 23: b = button08; break;
		case 24: b = button09; break;
		case 25: b = button10; break;
		case 26: b = button11; break;
		case 27: b = button12; break;
		case 28: b = button13; break;
		case 29: b = button14; break;
		case 30: b = button15; break;
		case 31: b = button16; break;
		case 32: b = button16; break;
		case  0: b = button17; break;
		case  1: b = button18; break;
		case  2: b = button19; break;
		case  3: b = button20; break;
		case  4: b = button21; break;
		case  5: b = button22; break;
		case  6: b = button23; break;
		case  7: b = button24; break;
		case  8: b = button25; break;
		case  9: b = button26; break;
		case 10: b = button27; break;
		case 11: b = button28; break;
		case 12: b = button29; break;
		case 13: b = button30; break;
		case 14: b = button31; break;
	}
	bitBlt(this,repaintRect->topLeft(),b,src);
}
