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
    m_pPixmapSLs = NULL;
    m_iNoPos = 0;
    m_iPos = 0;
    setNoPos(m_iNoPos);
}

WStatusLight::~WStatusLight()
{
    for (int i = 0; i < m_iNoPos; i++) {
        WPixmapStore::deletePixmap(m_pPixmapSLs[i]);
    }
    delete [] m_pPixmapSLs;
}

void WStatusLight::setNoPos(int iNoPos)
{
    // If pixmap array is already allocated, delete it
    if (m_pPixmapSLs != NULL) {
        for (int i = 0; i < m_iNoPos; i++) {
            WPixmapStore::deletePixmap(m_pPixmapSLs[i]);
        }
        delete [] m_pPixmapSLs;
    }

    if (iNoPos < 2)
        iNoPos = 2; //values less than 2 make no sense (need at least off, on)
    m_iNoPos = iNoPos;
    m_fValue = 0.;

    m_pPixmapSLs = new QPixmap*[m_iNoPos];
    for (int i = 0; i < m_iNoPos; ++i) {
        m_pPixmapSLs[i] = NULL;
    }
}

void WStatusLight::setup(QDomNode node)
{
    // Number of states. Add one to account for the background
    setNoPos(selectNodeInt(node, "NumberPos") + 1);

    // Set pixmaps
    for (int i = 0; i < m_iNoPos; ++i) {
        // Accept either PathStatusLight or PathStatusLight1 for value 1,
        QString nodeName = QString("PathStatusLight%1").arg(i);
        if (!selectNode(node, nodeName).isNull()) {
            setPixmap(i, getPath(selectNodeQString(node, nodeName)));
        } else if (i == 0 && !selectNode(node, "PathBack").isNull()) {
            setPixmap(i, getPath(selectNodeQString(node, "PathBack")));
        } else if (i == 1 && !selectNode(node, "PathStatusLight").isNull()) {
            setPixmap(i, getPath(selectNodeQString(node, "PathStatusLight")));
        } else {
            m_pPixmapSLs[i] = NULL;
        }
    }
}

void WStatusLight::setPixmap(int iState, const QString &filename)
{
    if (iState < 0 || iState >= m_iNoPos) {
        return;
    }
    int pixIdx = iState;

    QPixmap* pPixmap = WPixmapStore::getPixmap(filename);

    if (pPixmap != NULL && !pPixmap->isNull()) {
        m_pPixmapSLs[pixIdx] = pPixmap;

        // Set size of widget equal to pixmap size
        setFixedSize(pPixmap->size());
    } else {
        qDebug() << "WStatusLight: Error loading pixmap:" << filename << iState;
        WPixmapStore::deletePixmap(pPixmap);
        m_pPixmapSLs[pixIdx] = NULL;
    }
}

void WStatusLight::setValue(double v)
{
    int val = (int)v;
    if (m_iPos == val)
        return;

    if (m_iNoPos == 2) {
        //original behavior for two-state lights: any non-zero value is "on"
        m_iPos = val > 0 ? 1 : 0;
        update();
    } else {
        //multi-state behavior: values must be correct
        if (val < m_iNoPos && val >= 0) {
            m_iPos = val;
            update();
        } else {
            qDebug() << "Warning: wstatuslight asked for invalid position:" << val << "max val:" << m_iNoPos-1;
        }
    }
}

void WStatusLight::paintEvent(QPaintEvent *) {
    if (m_iPos < 0 || m_iPos >= m_iNoPos) {
        return;
    }

    QPainter p(this);
    if (m_pPixmapSLs[m_iPos] != NULL && !m_pPixmapSLs[m_iPos]->isNull()) {
        p.drawPixmap(0, 0, *m_pPixmapSLs[m_iPos]);
    }
}
