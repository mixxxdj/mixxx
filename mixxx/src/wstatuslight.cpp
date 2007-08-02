/***************************************************************************
                          wstatuslight.cpp  -  description
                             -------------------
    begin                : Wed May 30 2007
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
			   (C) 2007 by John Sully (converted from WVumeter)
    email                : jsully@scs.ryerson.ca
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "wstatuslight.h"
#include "wpixmapstore.h"
//Added by qt3to4:
#include <QPaintEvent>
#include <QtDebug>
#include <QPixmap>

WStatusLight::WStatusLight(QWidget *parent, const char *name) : WWidget(parent,name)
{
    m_pPixmapBack = 0;
    m_pPixmapSL = 0;
    m_pPixmapBuffer = 0;
    setBackgroundMode(Qt::NoBackground);
}

WStatusLight::~WStatusLight()
{
    resetPositions();
}

void WStatusLight::setup(QDomNode node)
{
    WWidget::setup(node);

    // Set pixmaps
    bool bHorizontal = false;
    if (!selectNode(node, "Horizontal").isNull() && selectNodeQString(node, "Horizontal")=="true")
        bHorizontal = true;
    setPixmaps(getPath(selectNodeQString(node, "PathBack")), getPath(selectNodeQString(node, "PathStatusLight")), bHorizontal);
}

void WStatusLight::resetPositions()
{
    if (m_pPixmapBack)
    {
        WPixmapStore::deletePixmap(m_pPixmapBack);
        m_pPixmapBack = 0;
        WPixmapStore::deletePixmap(m_pPixmapSL);
        m_pPixmapSL = 0;
        WPixmapStore::deletePixmap(m_pPixmapBuffer);
        m_pPixmapBuffer = 0;
    }
}

void WStatusLight::setPixmaps(const QString &backFilename, const QString &vuFilename, bool bHorizontal)
{
    m_pPixmapBack = WPixmapStore::getPixmap(backFilename);
    if (!m_pPixmapBack || m_pPixmapBack->size()==QSize(0,0))
        qDebug() << "WStatusLight: Error loading back pixmap" << backFilename;
    m_pPixmapSL = WPixmapStore::getPixmap(vuFilename);
    if (!m_pPixmapSL || m_pPixmapSL->size()==QSize(0,0))
        qDebug() << "WStatusLight: Error loading statuslight pixmap" << vuFilename;

    m_pPixmapBuffer = new QPixmap(m_pPixmapBack->size());
        
    setFixedSize(m_pPixmapBack->size());
    m_bHorizontal = bHorizontal;
}

void WStatusLight::paintEvent(QPaintEvent *)
{
    if (m_pPixmapBack!=0 && m_pPixmapSL!=0)
    {
	
	// Draw back on buffer
        if(m_fValue == 0)
	    bitBlt(m_pPixmapBuffer, 0, 0, m_pPixmapBack);
	else
	    bitBlt(m_pPixmapBuffer, 0, 0, m_pPixmapSL);
        // Draw light on buffer
        //bitBlt(m_pPixmapBuffer, 0, 0, m_pPixmapSL, 0, 0, m_pPixmapSL->width(), m_pPixmapSL->height());
	
        // Draw buffer on screen                
        bitBlt(this, 0, 0, m_pPixmapBuffer);
    }
}



