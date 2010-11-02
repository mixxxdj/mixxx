/***************************************************************************
                          wdisplay.cpp  -  description
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

#include "wdisplay.h"
#include "wpixmapstore.h"

#include <QPainter>
#include <QPaintEvent>
#include <QtDebug>
#include <QPixmap>

WDisplay::WDisplay(QWidget * parent) : WWidget(parent)
{
    m_pPixmaps = 0;
    setPositions(0);
}

WDisplay::~WDisplay()
{
    resetPositions();
}

void WDisplay::setup(QDomNode node)
{
    // Number of states
    setPositions(selectNodeInt(node, "NumberStates"));

    // Load knob  pixmaps
    QString path = selectNodeQString(node, "Path");
    for (int i=0; i<m_iNoPos; ++i)
    {
        setPixmap(i, getPath(path.arg(i)));
    }
}

void WDisplay::setPositions(int iNoPos)
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

void WDisplay::resetPositions()
{
    if (m_pPixmaps)
    {
        for (int i=0; i<m_iNoPos; i++)
            if (m_pPixmaps[i])
                WPixmapStore::deletePixmap(m_pPixmaps[i]);

        //WPixmapStore::deletePixmap(m_pPixmaps);
        m_pPixmaps = 0;
    }
}

void WDisplay::setPixmap(int iPos, const QString &filename)
{
    m_pPixmaps[iPos] = WPixmapStore::getPixmap(filename);
    if (!m_pPixmaps[iPos])
        qDebug() << "WDisplay: Error loading pixmap" << filename;
    else
        setFixedSize(m_pPixmaps[iPos]->size());
}

void WDisplay::paintEvent(QPaintEvent *)
{
    if (m_pPixmaps>0)
    {
        int idx = (int)(m_fValue*(float)(m_iNoPos)/128.);
        // Range check
        if (idx>(m_iNoPos-1))
            idx = m_iNoPos-1;
        else if (idx<0)
            idx = 0;
        if (m_pPixmaps[idx]) {
            QPainter p(this);
            p.drawPixmap(0, 0, *m_pPixmaps[idx]);
        }
    }
}



