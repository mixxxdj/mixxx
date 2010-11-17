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
    m_pPixmapSLs = 0;
    m_iNoPos = 0;
    m_iPos = 0;
    
    setNoPos(0);
}

WStatusLight::~WStatusLight()
{
    for (int i = 0; i < m_iNoPos; i++) {
        WPixmapStore::deletePixmap(m_pPixmapSLs[i]);
    }
}

void WStatusLight::setNoPos(int iNoPos)
{
    m_iNoPos = iNoPos;
    m_fValue = 0.;

    // If pixmap array is already allocated, delete it
    if (m_pPixmapSLs)
        delete [] m_pPixmapSLs;

    if (m_iNoPos>0)
    {
        m_pPixmapSLs = new QPixmap*[m_iNoPos];
        for (int i=0; i<m_iNoPos; i++)
            m_pPixmapSLs[i] = 0;
    }
}

void WStatusLight::setup(QDomNode node)
{
	// Number of states
    m_iNoPos = selectNodeInt(node, "NumberPos") + 1;
    setNoPos(m_iNoPos);
	
    // Set pixmaps
    m_bHorizontal = false;
    if (!selectNode(node, "Horizontal").isNull() && selectNodeQString(node, "Horizontal")=="true")
        m_bHorizontal = true;
	
	for (int i=0; i<m_iNoPos; i++)
    {
    	switch(i)
    	{
    		case 0:
	    		// Set background pixmap if available
				if (!selectNode(node, "PathBack").isNull())
					setPixmap(0, getPath(selectNodeQString(node, "PathBack")));
			   	else
			   		m_pPixmapSLs[0] = 0;
	    		break;
	    	case 1:
	    		setPixmap(1, getPath(selectNodeQString(node, "PathStatusLight")));
	    		break;
	    	default:
	    		setPixmap(i, getPath(selectNodeQString(node, QString("PathStatusLight%1").arg(i))));
	    }
	}
}

void WStatusLight::setPixmap(int iState, const QString &filename)
{
    int pixIdx = iState;
    m_pPixmapSLs[pixIdx] = WPixmapStore::getPixmap(filename);
    if (!m_pPixmapSLs[pixIdx] || m_pPixmapSLs[pixIdx]->size()==QSize(0,0))
        qDebug() << "WPushButton: Error loading pixmap:" << filename << iState;

    // Set size of widget equal to pixmap size
    setFixedSize(m_pPixmapSLs[pixIdx]->size());
}

void WStatusLight::setValue(double v)
{
	//FIXME: why are we getting invald values here?
	if (m_iPos != (int)v && (int)v < m_iNoPos && (int)v >= 0)
	{
	    m_iPos = (int)v;
	    update();
	}
}

void WStatusLight::paintEvent(QPaintEvent *)
{
	Q_ASSERT (m_iPos < m_iNoPos);
    if (m_pPixmapSLs[m_iPos])
    {
        QPainter p(this);
        if(m_iPos != 0 && m_pPixmapSLs[0]) p.drawPixmap(0, 0, *m_pPixmapSLs[0]);
        p.drawPixmap(0, 0, *m_pPixmapSLs[m_iPos]);
    }
}
