/***************************************************************************
                          wvonylcontrolindicator.cpp  -  description
                             -------------------
    begin                : Fri Jul 22 2003
    copyright            : (C) 2007 by Albert Santoni
                                           (C) 2003 by Tue & Ken Haste Andersen
    email                : albert [at] santoni *dot* ca
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "wvinylcontrolindicator.h"
#include "wpixmapstore.h"

WVinylControlIndicator::WVinylControlIndicator(QWidget * parent, const char * name) : WWidget(parent,name)
{
    m_pPixmapBack = 0;
    //m_pPixmapVu = 0;
    m_pPixmapBuffer = 0;
    iWidth = 30;
    iHeight = 40;
    iRadius = 10;
    setBackgroundMode(Qt::NoBackground);
    paintTimer = new QTimer(this);
    connect(paintTimer, SIGNAL(timeout()), this, SLOT(paintEvent()));
    paintTimer->start(80); //~12 fps
}

WVinylControlIndicator::~WVinylControlIndicator()
{
    resetPositions();

    if (paintTimer)
    {
        paintTimer->stop();
        delete paintTimer;
        paintTimer = NULL;
    }
}

void WVinylControlIndicator::setup(QDomNode node)
{
    WWidget::setup(node);

    iWidth = selectNodeInt(node, "Width");
    iHeight = selectNodeInt(node, "Height");
    iRadius = selectNodeInt(node, "Radius");

    // Set pixmaps
    /*bool bHorizontal = false;
       if (!selectNode(node, "Horizontal").isNull() && selectNodeQString(node, "Horizontal")=="true")
        bHorizontal = true;
     */
    setPixmaps(getPath(selectNodeQString(node, "PathBack")), getPath(selectNodeQString(node, "PathVu")));
}

void WVinylControlIndicator::resetPositions()
{
    if (m_pPixmapBack)
    {
        WPixmapStore::deletePixmap(m_pPixmapBack);
        m_pPixmapBack = 0;
        //WPixmapStore::deletePixmap(m_pPixmapVu);
        //m_pPixmapVu = 0;
        WPixmapStore::deletePixmap(m_pPixmapBuffer);
        m_pPixmapBuffer = 0;
    }
}

void WVinylControlIndicator::setPixmaps(const QString &backFilename, const QString &vuFilename)
{
    m_pPixmapBack = WPixmapStore::getPixmap(backFilename);
    if (!m_pPixmapBack || m_pPixmapBack->size()==QSize(0,0))
        qDebug("WVinylControlIndicator: Error loading back pixmap %s",backFilename.latin1());

    /*
       m_pPixmapVu = WPixmapStore::getPixmap(vuFilename);
       if (!m_pPixmapVu || m_pPixmapVu->size()==QSize(0,0))
        qDebug("WVinylControlIndicator: Error loading vu pixmap %s",vuFilename.latin1());
     */

    m_pPixmapBuffer = new QPixmap(m_pPixmapBack->size());

    setFixedSize(m_pPixmapBack->size());
    //m_bHorizontal = bHorizontal;

    /*if (m_bHorizontal)
        m_iNoPos = m_pPixmapVu->width();
       else
        m_iNoPos = m_pPixmapVu->height();
     */
}

void WVinylControlIndicator::paintEvent()
{
    if (m_pPixmapBack!=0)
    {
        int idx = (int)(m_fValue*(float)(m_iNoPos)/128.);
        static int phase = 0;

        // Range check
        /*     if (idx>m_iNoPos)
                 idx = m_iNoPos;
             else if (idx<0)
                 idx = 0;
         */

        // Draw back on buffer
        bitBlt(m_pPixmapBuffer, 0, 0, m_pPixmapBack);

        //qDebug("PAINTING WVinylControlIndicator!");
        QPen tip(black, 4);
        QPen tail(black, 1);

        QPainter p(m_pPixmapBuffer);
        p.setBrush(Qt::black);
        p.setPen(tip);
        //for (int i = 0; i < iWidth; i++)
        p.drawPoint(iWidth/2 + iRadius*sin(phase*(2*6.238)/360), iHeight/2 + iRadius*cos(phase*(2*6.238)/360));
        p.setPen(tail);
        p.drawPoint(iWidth/2 + iRadius*sin((phase-2)*(2*6.238)/360), iHeight/2 + iRadius*cos((phase-2)*(2*6.238)/360));
        p.drawPoint(iWidth/2 + iRadius*sin((phase-4)*(2*6.238)/360), iHeight/2 + iRadius*cos((phase-4)*(2*6.238)/360));
        p.end();

        phase++; if (phase > 360) phase = 0;


        // Draw (part of) vu on buffer
        /*if (m_bHorizontal)
            bitBlt(m_pPixmapBuffer, 0, 0, m_pPixmapVu, 0, 0, idx, m_pPixmapVu->height());
           else*/
        //bitBlt(m_pPixmapBuffer, 0, m_iNoPos-idx, m_pPixmapVu, 0, m_iNoPos-idx, m_pPixmapVu->width(), idx);

        // Draw buffer on screen
        bitBlt(this, 0, 0, m_pPixmapBuffer);
    }
}



