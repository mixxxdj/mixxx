/***************************************************************************
                          wpushbutton.cpp  -  description
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

#include "wpushbutton.h"

WPushButton::WPushButton(QWidget *parent, const char *name ) : WWidget(parent,name)
{
    m_pPixmaps = 0;
    m_pPixmapBack = 0;
    m_pPixmapBuffer = 0;
    setStates(0);

    setBackgroundMode(NoBackground);
}

WPushButton::~WPushButton()
{
}

void WPushButton::setStates(int iStates)
{
    m_iNoStates = iStates;
    m_iState = 0;
    m_bPressed = false;
    
    // If pixmap array is already allocated, delete it
    if (m_pPixmaps)
        delete m_pPixmaps;
    
    if (iStates>0)
    {
        m_pPixmaps = new QPixmap*[2*m_iNoStates];
        for (int i=0; i<2*m_iNoStates; ++i)
            m_pPixmaps[i] = 0;
    }
}

void WPushButton::setPixmap(int iState, bool bPressed, const QString &filename)
{

    int pixIdx = (iState*2)+bPressed;
    m_pPixmaps[pixIdx] = new QPixmap(filename);
    if (!m_pPixmaps[pixIdx])
        qDebug("WPushButton: Error loading pixmap: %s",filename.latin1());

}

void WPushButton::setPixmapBackground(const QString &filename)
{
    // Load background pixmap
    m_pPixmapBack = new QPixmap(filename);
    if (!m_pPixmapBack)
        qDebug("WPushButton: Error loading background pixmap: %s",filename.latin1());

    // Construct corresponding double buffer
    m_pPixmapBuffer = new QPixmap(m_pPixmapBack->size());
}

void WPushButton::setValue(double v)
{
    m_iState = (int)v;

    update();
}

void WPushButton::paintEvent(QPaintEvent *)
{
    if (m_iNoStates>0)
    {
        int idx = ((m_iState%m_iNoStates)*2)+m_bPressed;
        if (m_pPixmaps[idx])
        {

            // If m_pPixmapBuffer is defined, use double buffering when painting,
            // otherwise paint the button directly to the screen.
            if (m_pPixmapBuffer!=0)
            {
                // Paint background on buffer
                bitBlt(m_pPixmapBuffer, 0, 0, m_pPixmapBack);

                // Paint button on buffer
                bitBlt(m_pPixmapBuffer, 0, 0, m_pPixmaps[idx]);
                
                // Paint buffer to screen
                bitBlt(this, 0, 0, m_pPixmapBuffer);
            }
            else
                bitBlt(this, 0, 0, m_pPixmaps[idx]);
        }
    }
}

void WPushButton::mousePressEvent(QMouseEvent *e)
{
    m_bPressed = true;

    // Calculate new state if the left mouse button was used to press the button
    if (e->button()==Qt::LeftButton)
    {
        // Special case if it is a one state button.
        if (m_iNoStates==1)
        {
            if (m_iState==0)
                m_iState = 1;
            else
                m_iState = 0;
        }
        else
            m_iState = (m_iState+1)%m_iNoStates;
    }
    
    if (e->button()==Qt::LeftButton)
        emit(valueChangedLeftDown((double)m_iState));
    else if (e->button()==Qt::RightButton)
        emit(valueChangedRightDown((double)m_iState));

    update();
}

void WPushButton::mouseReleaseEvent(QMouseEvent *e)
{
    m_bPressed = false;

    // Update state if left button and it is a one state button.
    if (m_iNoStates==1 && e->button()==Qt::LeftButton)
    {
        if (m_iState==0)
            m_iState = 1;
        else
            m_iState = 0;
    }


    if (e->button()==Qt::LeftButton)
        emit(valueChangedLeftUp((double)m_iState));
    else if (e->button()==Qt::RightButton)
        emit(valueChangedRightUp((double)m_iState));

    update();
}
