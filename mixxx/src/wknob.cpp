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

WKnob::WKnob(QWidget *parent, const char *name) : WWidget(parent,name)
{
    m_pPixmaps = 0;
    setPositions(0);
    setBackgroundMode(NoBackground);
}

WKnob::~WKnob()
{
    resetPositions();
}

void WKnob::setPositions(int iNoPos)
{
    m_iNoPos = iNoPos;
    m_iPos = 0;

    resetPositions();

    if (m_iNoPos>0)
    {
        m_pPixmaps = new QPixmap*[m_iNoPos];
        for (int i=0; i<m_iNoPos; i++)
            m_pPixmaps[i] = 0;
    }
}

void WKnob::resetPositions()
{
    if (m_pPixmaps)
    {
        for (int i=0; i<m_iNoPos; i++)
            if (m_pPixmaps[i])
                delete m_pPixmaps[i];

        delete m_pPixmaps;
        m_pPixmaps = 0;
    }
}

void WKnob::setPixmap(int iPos, const QString &filename)
{
    m_pPixmaps[iPos] = new QPixmap(filename);
    if (!m_pPixmaps[iPos])
        qDebug("WKnob: Error loading pixmap %s",filename.latin1());
}

void WKnob::mouseMoveEvent(QMouseEvent *e)
{
    if (e->button()==Qt::LeftButton)
    {
        m_fOldValue = m_fValue;
        m_fValue = (e->y()-m_fStartValue+64);
        if (m_fValue>127)
            m_fValue = 127;
        else if (m_fValue<0)
            m_fValue = 0;

        emit(valueChangedLeftUp(m_fValue));
    }
    else if (e->button()==Qt::RightButton)
        emit(valueChangedRightUp(m_fValue));

    update();
}

void WKnob::mousePressEvent(QMouseEvent *e)
{
    m_fStartValue = e->y();

    if (e->button()==Qt::LeftButton)
        emit(valueChangedLeftDown(m_fValue));
    else if (e->button()==Qt::RightButton)
        emit(valueChangedRightDown(m_fValue));
}

void WKnob::mouseReleaseEvent(QMouseEvent *e)
{
    m_fOldValue = m_fValue;
    m_fValue = 64;

    if (e->button()==Qt::LeftButton)
        emit(valueChangedLeftUp(m_fValue));
    else if (e->button()==Qt::RightButton)
        emit(valueChangedRightUp(m_fValue));

    update();
}

void WKnob::paintEvent(QPaintEvent *)
{
    if (m_pPixmaps>0)
    {
        int idx = (int)((m_fValue-64.)*((m_iNoPos-1.)/127.))+(m_iNoPos/2.);

        // Range check
        if (idx>(m_iNoPos-1))
            idx = m_iNoPos-1;
        else if (idx<0)
            idx = 0;

        bitBlt(this, 0, 0, m_pPixmaps[idx]);
    }
}
