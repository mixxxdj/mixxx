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
#include "wpixmapstore.h"

WPushButton::WPushButton(QWidget *parent, const char *name ) : WWidget(parent,name)
{
    m_pPixmaps = 0;
    m_pPixmapBack = 0;
    m_pPixmapBuffer = 0;
    setStates(0);

    setBackgroundMode(Qt::NoBackground);
}

WPushButton::~WPushButton()
{
}

void WPushButton::setup(QDomNode node)
{
    // Number of states
    setStates(selectNodeInt(node, "NumberStates"));

    // Set background pixmap if available
    if (!selectNode(node, "BackPath").isNull())
        setPixmapBackground(getPath(selectNodeQString(node, "BackPath")));

    // Load pixmaps for associated states
    QDomNode state = selectNode(node, "State");
    while (!state.isNull())
    {
        if (state.isElement() && state.nodeName() == "State")
        {
            setPixmap(selectNodeInt(state, "Number"), true, getPath(selectNodeQString(state, "Pressed")));
            setPixmap(selectNodeInt(state, "Number"), false, getPath(selectNodeQString(state, "Unpressed")));
        }
        state = state.nextSibling();
    }

    // Setup position
    WWidget::setup(node);
}

void WPushButton::setStates(int iStates)
{
    m_iNoStates = iStates;
    m_fValue = 0.;
    m_bPressed = false;

    // If pixmap array is already allocated, delete it
    if (m_pPixmaps)
        delete [] m_pPixmaps;

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
    m_pPixmaps[pixIdx] = WPixmapStore::getPixmap(filename);
    if (!m_pPixmaps[pixIdx])
        qDebug("WPushButton: Error loading pixmap: %s",filename.latin1());

    // Set size of widget equal to pixmap size
    setFixedSize(m_pPixmaps[pixIdx]->size());
}

void WPushButton::setPixmapBackground(const QString &filename)
{
    // Load background pixmap
    m_pPixmapBack = WPixmapStore::getPixmap(filename);
    if (!m_pPixmapBack)
        qDebug("WPushButton: Error loading background pixmap: %s",filename.latin1());

    // Construct corresponding double buffer
    m_pPixmapBuffer = new QPixmap(m_pPixmapBack->size());
}

void WPushButton::setValue(double v)
{
    m_fValue = v;

    if (m_iNoStates==1)
    {
        if (m_fValue==1.)
            m_bPressed = true;
        else
            m_bPressed = false;
    }

    update();
}

void WPushButton::paintEvent(QPaintEvent *)
{
    if (m_iNoStates>0)
    {
        int idx = (((int)m_fValue%m_iNoStates)*2)+m_bPressed;
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

    // Calculate new state if it is a one state button
    if (m_iNoStates==1)
    {
        if (m_fValue==0.)
            m_fValue = 1.;
        else
            m_fValue = 0.;
    }
    // Update state on left press if it is a n-state button
    else if (e->button()==Qt::LeftButton)
    {
        m_fValue = (int)(m_fValue+1.)%m_iNoStates;
    }

    if (e->button()==Qt::LeftButton)
        emit(valueChangedLeftDown((double)m_fValue));
    else if (e->button()==Qt::RightButton)
        emit(valueChangedRightDown((double)m_fValue));

    update();
}

void WPushButton::mouseReleaseEvent(QMouseEvent *e)
{
    m_bPressed = false;

    // Update state if it is a one state button.
    if (m_iNoStates==1) // && e->button()==Qt::LeftButton)
    {
        if (m_fValue==0.)
            m_fValue = 1.;
        else
            m_fValue = 0.;
    }


    if (e->button()==Qt::LeftButton)
        emit(valueChangedLeftUp((double)m_fValue));
    else if (e->button()==Qt::RightButton)
        emit(valueChangedRightUp((double)m_fValue));

    update();
}
