/***************************************************************************
                          wslidercomposed.cpp  -  description
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

#include "wslidercomposed.h"
#include <qpixmap.h>
#include <qpainter.h>
#include "defs.h"


WSliderComposed::WSliderComposed(QWidget *parent, const char *name ) : WWidget(parent,name)
{
    m_pSlider = 0;
    m_pHandle = 0;
    m_pDoubleBuffer = 0;

    // Set default values
    m_iSliderLength=0;
    m_iHandleLength=0;

    m_bReverse = false;
    m_fValue = 63.;
}

WSliderComposed::~WSliderComposed()
{
    unsetPixmaps();
}

void WSliderComposed::setPixmaps(bool bHorizontal, const QString &filenameSlider, const QString &filenameHandle)
{
    m_bHorizontal = bHorizontal;
    unsetPixmaps();
    m_pSlider = new QPixmap(filenameSlider);
    if (!m_pSlider)
        qDebug("WSliderComposed: Error loading slider pixmap: %s",filenameSlider.latin1());
    m_pHandle = new QPixmap(filenameHandle);
    if (!m_pHandle)
        qDebug("WSliderComposed: Error loading handle pixmap: %s",filenameHandle.latin1());
    m_pDoubleBuffer = new QPixmap(m_pSlider->size());
    
    if (m_bHorizontal)
    {
        m_iSliderLength = m_pSlider->width();
        m_iHandleLength = m_pHandle->width();
    }
    else
    {
        m_iSliderLength = m_pSlider->height();
        m_iHandleLength = m_pHandle->height();
    }

    setValue(m_fValue);

    repaint();
}

void WSliderComposed::unsetPixmaps()
{
    if (m_pSlider)
        delete m_pSlider;
    if (m_pHandle)
        delete m_pHandle;
    if (m_pDoubleBuffer)
        delete m_pDoubleBuffer;
    m_pSlider = 0;
    m_pHandle = 0;
    m_pDoubleBuffer = 0;
}

void WSliderComposed::mouseMoveEvent(QMouseEvent *e)
{
    if (m_bHorizontal)
        m_iPos = e->x()-m_iHandleLength/2;
    else
        m_iPos = e->y()-m_iHandleLength/2;

    if (m_iPos>(m_iSliderLength-m_iHandleLength))
        m_iPos = m_iSliderLength-m_iHandleLength;
    else if (m_iPos<0)
        m_iPos = 0;

    // value ranges from 0 to 127
    m_fValue = (double)m_iPos*(127./(double)(m_iSliderLength-m_iHandleLength));
    if ((!m_bHorizontal && !m_bReverse) || (m_bHorizontal && m_bReverse))
        m_fValue = 127.-m_fValue;

    // Emit valueChanged signal
    if (e->button()==Qt::RightButton)
        emit(valueChangedRightUp(m_fValue));
    else
        emit(valueChangedLeftUp(m_fValue));

    // Update display
    update();
}


void WSliderComposed::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::RightButton)
        reset();
    else
        mouseMoveEvent(e);
}

void WSliderComposed::paintEvent(QPaintEvent *)
{
    if (m_pSlider && m_pHandle)
    {
        int posx;
        int posy;
        if (m_bHorizontal)
        {
            posx = m_iPos;
            posy = 0;
        }
        else
        {
            posx = 0;
            posy = m_iPos;
        }

        // Draw slider followed by handle to double buffer
        bitBlt(m_pDoubleBuffer, 0, 0, m_pSlider);
        bitBlt(m_pDoubleBuffer, posx, posy, m_pHandle);

        // Draw double buffer to screen
        bitBlt(this, 0, 0, m_pDoubleBuffer);
    }
}

int WSliderComposed::getReverse()
{
    if (m_bReverse)
        return 1;
    else
        return 0;
}

void WSliderComposed::setValue(double fValue)
{
    // Set value without emitting a valueChanged signal, and force display update
    m_fValue = fValue;

    // Calculate handle position
    if ((!m_bHorizontal && !m_bReverse) || (m_bHorizontal && m_bReverse))
        fValue = 127-fValue;
    m_iPos = (int)((fValue/127.)*(double)(m_iSliderLength-m_iHandleLength));

    repaint();
}

void WSliderComposed::slotSetReverse(int iReverse)
{
    if (iReverse==0)
        m_bReverse = false;
    else
        m_bReverse = true;
        
    reset();    
}

void WSliderComposed::reset()
{
    setValue(63.);
    emit(valueChangedLeftUp(m_fValue));
}

