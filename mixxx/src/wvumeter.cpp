/***************************************************************************
                          wvumeter.cpp  -  description
                             -------------------
    begin                : Fri Jul 22 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
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

#include "wvumeter.h"
#include "wpixmapstore.h"

WVuMeter::WVuMeter(QWidget *parent, const char *name) : WWidget(parent,name)
{
    m_pPixmapBack = 0;
    m_pPixmapVu = 0;
    m_pPixmapBuffer = 0;
    setBackgroundMode(NoBackground);
}

WVuMeter::~WVuMeter()
{
    resetPositions();
}

void WVuMeter::setup(QDomNode node)
{
    WWidget::setup(node);

    // Set pixmaps
    bool bHorizontal = false;
    if (!selectNode(node, "Horizontal").isNull() && selectNodeQString(node, "Horizontal")=="true")
        bHorizontal = true;
    setPixmaps(getPath(selectNodeQString(node, "PathBack")), getPath(selectNodeQString(node, "PathVu")), bHorizontal);
}

void WVuMeter::resetPositions()
{
    if (m_pPixmapBack)
    {
        delete m_pPixmapBack;
        m_pPixmapBack = 0;
        delete m_pPixmapVu;
        m_pPixmapVu = 0;
        delete m_pPixmapBuffer;
        m_pPixmapBuffer = 0;
    }
}

void WVuMeter::setPixmaps(const QString &backFilename, const QString &vuFilename, bool bHorizontal)
{
    m_pPixmapBack = WPixmapStore::getPixmap(backFilename);
    if (!m_pPixmapBack || m_pPixmapBack->size()==QSize(0,0))
        qDebug("WVuMeter: Error loading back pixmap %s",backFilename.latin1());
    m_pPixmapVu = WPixmapStore::getPixmap(vuFilename);
    if (!m_pPixmapVu || m_pPixmapVu->size()==QSize(0,0))
        qDebug("WVuMeter: Error loading vu pixmap %s",vuFilename.latin1());

    m_pPixmapBuffer = new QPixmap(m_pPixmapBack->size());
        
    setFixedSize(m_pPixmapBack->size());
    m_bHorizontal = bHorizontal;
    if (m_bHorizontal)
        m_iNoPos = m_pPixmapVu->width();
    else
        m_iNoPos = m_pPixmapVu->height();    
}

void WVuMeter::paintEvent(QPaintEvent *)
{
    if (m_pPixmapBack!=0)
    {
        int idx = (int)(m_fValue*(float)(m_iNoPos)/128.);

        // Range check
        if (idx>m_iNoPos)
            idx = m_iNoPos;
        else if (idx<0)
            idx = 0;

        // Draw back on buffer
        bitBlt(m_pPixmapBuffer, 0, 0, m_pPixmapBack);

        // Draw (part of) vu on buffer
        if (m_bHorizontal)
            bitBlt(m_pPixmapBuffer, 0, 0, m_pPixmapVu, 0, 0, idx, m_pPixmapVu->height());
        else
            bitBlt(m_pPixmapBuffer, 0, m_iNoPos-idx, m_pPixmapVu, 0, m_iNoPos-idx, m_pPixmapVu->width(), idx);
                    
        // Draw buffer on screen                
        bitBlt(this, 0, 0, m_pPixmapBuffer);
    }
}



