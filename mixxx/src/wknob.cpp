/***************************************************************************
                          wknob.cpp  -  description
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

#include "wknob.h"

#include "images/knob.cpp"

// Static member variable definition
QPixmap **WKnob::pix = 0;
int WKnob::instantiateNo = 0;

WKnob::WKnob(QWidget *parent, const char *name ) : QDial(0,127,1,64,parent,name)
{
    instantiateNo++;
    
    // Convert xpm's to pixmaps
    if (instantiateNo==1)
    {
        pix = new QPixmap*[32];
        for (int i=0; i<32; i++)
            pix[i] = new QPixmap(knob[i]);
    }
    setBackgroundMode(NoBackground);
}

WKnob::~WKnob()
{
    instantiateNo--;
    
    if (instantiateNo==0)
    {
        for (int i=0; i<32; i++)
            delete pix[i];
        delete [] pix;
    }
}

QPixmap *WKnob::getKnob()
{
    int range = (maxValue()-minValue());
    int val = value()*32/range;
    if (val==32)
        val=15;
    else if (val==0)
        val=17;
    else
        val = (val+16)%32;

    return pix[val];
}

void WKnob::repaintScreen(const QRect *cr)
{
    repaintScreen(cr,getKnob());
}

void WKnob::repaintScreen(const QRect *cr, QPixmap *p)
{
    QRect *repaintRect;
    if (cr != 0)
        repaintRect = new QRect(cr->topLeft(),cr->size());
    else
        repaintRect = new QRect(0,0,width(),height());

//    qDebug("topleft: (%i,%i), size (%i,%i)",repaintRect->topLeft().x(), repaintRect->topLeft().y(),
//                                            repaintRect->size().width(), repaintRect->size().height());

    bitBlt(this,repaintRect->topLeft(),p,QRect(repaintRect->topLeft(),repaintRect->size()));

    delete repaintRect;
}


