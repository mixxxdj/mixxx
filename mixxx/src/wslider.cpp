/***************************************************************************
                          wslider.cpp  -  description
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

#include "wslider.h"
#include <qpixmap.h>
#include <qpainter.h>
#include "defs.h"

#include "images/slider_small_v.xpm"
#include "images/slider_small_v_handle.xpm"
#include "images/slider_mid_v.xpm"
#include "images/slider_mid_v_handle.xpm"
#include "images/slider_large_h.xpm"
#include "images/slider_large_h_handle.xpm"

// Static member variable definition
QPixmap *WSlider::smallv   = 0;
QPixmap *WSlider::smallv_h = 0;
QPixmap *WSlider::midv     = 0;
QPixmap *WSlider::midv_h   = 0;
QPixmap *WSlider::largeh   = 0;
QPixmap *WSlider::largeh_h = 0;

WSlider::WSlider(QWidget *parent, const char *name ) : QWidget(parent,name)
{
    // Initialize static pointers to pixmaps
    if (smallv == 0)
    {
        smallv   = new QPixmap(slider_small_v_xpm);
        smallv_h = new QPixmap(slider_small_v_handle_xpm);
        midv     = new QPixmap(slider_mid_v_xpm);
        midv_h   = new QPixmap(slider_mid_v_handle_xpm);
        largeh   = new QPixmap(slider_large_h_xpm);
        largeh_h = new QPixmap(slider_large_h_handle_xpm);
    }

    // Set default value
    setDefaultValue();
}

WSlider::~WSlider()
{
    delete smallv; delete smallv_h;
}

void WSlider::mouseMoveEvent(QMouseEvent *e)
{
    if (size_state < 3)
        pos = e->y();
    else
        pos = e->x();
    if (pos>slider_length-(handle_length/2))
        pos = slider_length-(handle_length/2);
    else if (pos<(handle_length/2))
        pos = handle_length/2;

    // value ranges from 0 to 127
    value = (int)((FLOAT_TYPE)pos*(128./(FLOAT_TYPE)(slider_length)));
    if (size_state==2)
        value = 127.-value;

    //qDebug("WSlider value: %i",value);

    // Emit valueChanged signal
    emit(valueChanged(value));

    // Update display
    update();
}


void WSlider::mousePressEvent(QMouseEvent *e)
{
    mouseMoveEvent(e);
}

void WSlider::paintEvent(QPaintEvent *)
{
    QPainter paint(this);

    // Ensure size_state has been set
    if (size_state == 0)
        if (minimumWidth()==20 & minimumHeight()==80)
        {
            size_state = 1;
            slider_length = 80;
            handle_length = 10;
        }
        else if (minimumWidth()==30 & minimumHeight()==170)
        {
            size_state = 2;
            slider_length = 170;
            handle_length = 16;
        }
        else if (minimumWidth()==400 & minimumHeight()==37)
        {
            size_state = 3;
            slider_length = 400;
            handle_length = 16;
        }

    if (pos==-1)
        setValue(value);

    if (size_state == 1)
    {
        paint.drawPixmap(0,0,*smallv);
        paint.drawPixmap(0,pos-handle_length/2,*smallv_h);
    }
    else if (size_state == 2)
    {
        paint.drawPixmap(0,0,*midv);
        paint.drawPixmap(0,pos-handle_length/2,*midv_h);
    }
    else if (size_state == 3)
    {
        paint.drawPixmap(0,0,*largeh);
        paint.drawPixmap(pos-handle_length/2,0,*largeh_h);
    }
}

void WSlider::setValue(int v)
{
    // Set value without emitting a valueChanged signal, and force display update
    value = v;

    // Calculate handle position
    if (size_state==2)
        v = 127.-v;
    pos = (int)(((FLOAT_TYPE)(v)/128.)*(FLOAT_TYPE)(slider_length-handle_length))+handle_length/2;
    if (size_state==0)
        pos=0;
    repaint();
    emit(valueChanged(value));
}

void WSlider::slotSetPosition(int p)
{
    pos = p;
    value = (int)((FLOAT_TYPE)p*(128./100.));

    repaint();
    emit(valueChanged(value));
}


void WSlider::setDefaultValue()
{
    value = 63;
    pos = -1;
    size_state=0;
    slider_length=0;
    handle_length=0;
}