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

#include <QPaintEvent>
#include <QPainter>
#include <QtDebug>
#include <QPixmap>

WStatusLight::WStatusLight(QWidget * parent) : WWidget(parent)
{
    m_pPixmapBack = 0;
    m_pPixmapSL = 0;
}

WStatusLight::~WStatusLight()
{
    resetPositions();
}

void WStatusLight::setup(QDomNode node)
{
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

    setFixedSize(m_pPixmapBack->size());
    m_bHorizontal = bHorizontal;
}

void WStatusLight::paintEvent(QPaintEvent *)
{
    if (m_pPixmapBack!=0 && m_pPixmapSL!=0)
    {
        QPainter p(this);

        if(m_fValue == 0)
            p.drawPixmap(0, 0, *m_pPixmapBack);
        else
            p.drawPixmap(0, 0, *m_pPixmapSL);
    }
}
