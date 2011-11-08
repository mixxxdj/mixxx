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

    // Set background pixmap if available
    if (!selectNode(node, "PathBack").isNull()) {
        setPixmap(0, getPath(selectNodeQString(node, "PathBack")));
    } else {
        m_pPixmapSLs[0] = NULL;
    }

    // Set pixmaps
    for (int i = 1; i < m_iNoPos; ++i) {
        QString nodeName = (i == 1) ? "PathStatusLight" : QString("PathStatusLight%1").arg(i);
        if (!selectNode(node, nodeName).isNull()) {
            setPixmap(i, getPath(selectNodeQString(node, nodeName)));
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
        if (val == 0)
            m_iPos = 0;
        else
            m_iPos = 1;
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

void WStatusLight::paintEvent(QPaintEvent *)
{
    Q_ASSERT (m_iPos < m_iNoPos);

    QPainter p(this);

    // Draw the background
    if (m_iPos != 0 && m_iNoPos > 0 && m_pPixmapSLs[0] != NULL && !m_pPixmapSLs[0]->isNull()) {
        p.drawPixmap(0, 0, *m_pPixmapSLs[0]);
    }

    if (m_iPos >= 0 && m_iPos < m_iNoPos && m_pPixmapSLs[m_iPos] != NULL && !m_pPixmapSLs[m_iPos]->isNull()) {
        p.drawPixmap(0, 0, *m_pPixmapSLs[m_iPos]);
    }
}
