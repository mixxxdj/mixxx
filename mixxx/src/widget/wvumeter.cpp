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
//Added by qt3to4:
#include <QPaintEvent>
#include <QtGui>
#include <QtCore>
#include <QtDebug>
#include <QPixmap>

#define DEFAULT_FALLTIME 20
#define DEFAULT_FALLSTEP 1
#define DEFAULT_HOLDTIME 400
#define DEFAULT_HOLDSIZE 5

#include "util/timer.h"

WVuMeter::WVuMeter(QWidget * parent) : WWidget(parent)
{
    m_pPixmapBack = 0;
    m_pPixmapVu = 0;
    m_iPeakHoldSize = m_iPeakPos = 0;
    m_iPeakHoldCountdown = 0;
    m_iNoPos = 0;
}

WVuMeter::~WVuMeter()
{
    resetPositions();
}

void WVuMeter::setup(QDomNode node)
{
    // Set pixmaps
    bool bHorizontal = false;
    if (!selectNode(node, "Horizontal").isNull() &&
        selectNodeQString(node, "Horizontal")=="true")
        bHorizontal = true;

    setPixmaps(getPath(selectNodeQString(node, "PathBack")),
               getPath(selectNodeQString(node, "PathVu")), bHorizontal);

    m_iPeakHoldSize = selectNodeInt(node, "PeakHoldSize");
    if(m_iPeakHoldSize < 0 || m_iPeakHoldSize > 100)
        m_iPeakHoldSize = DEFAULT_HOLDSIZE;

    m_iPeakFallStep = selectNodeInt(node, "PeakFallStep");
    if(m_iPeakFallStep < 1 || m_iPeakFallStep > 1000)
        m_iPeakFallStep = DEFAULT_FALLSTEP;

    m_iPeakHoldTime = selectNodeInt(node, "PeakHoldTime");
    if(m_iPeakHoldTime < 1 || m_iPeakHoldTime > 3000)
        m_iPeakHoldTime = DEFAULT_HOLDTIME;

    m_iPeakFallTime = selectNodeInt(node, "PeakFallTime");
    if(m_iPeakFallTime < 1 || m_iPeakFallTime > 1000)
        m_iPeakFallTime = DEFAULT_FALLTIME;
}

void WVuMeter::resetPositions()
{
    if (m_pPixmapBack)
    {
        WPixmapStore::deletePixmap(m_pPixmapBack);
        m_pPixmapBack = 0;
        WPixmapStore::deletePixmap(m_pPixmapVu);
        m_pPixmapVu = 0;
    }
}

void WVuMeter::setPixmaps(const QString &backFilename, const QString &vuFilename, bool bHorizontal)
{
    m_pPixmapBack = WPixmapStore::getPixmap(backFilename);
    if (!m_pPixmapBack || m_pPixmapBack->size()==QSize(0,0))
        qDebug() << "WVuMeter: Error loading back pixmap" << backFilename;
    m_pPixmapVu = WPixmapStore::getPixmap(vuFilename);
    if (!m_pPixmapVu || m_pPixmapVu->size()==QSize(0,0))
        qDebug() << "WVuMeter: Error loading vu pixmap" << vuFilename;

    setFixedSize(m_pPixmapBack->size());
    m_bHorizontal = bHorizontal;
    if (m_bHorizontal)
        m_iNoPos = m_pPixmapVu->width();
    else
        m_iNoPos = m_pPixmapVu->height();
}

void WVuMeter::setValue(double fValue)
{
    int idx = (int)(fValue * (float)(m_iNoPos)/128.);
    // Range check
    if (idx>m_iNoPos)
        idx = m_iNoPos;
    else if (idx<0)
        idx = 0;

    setPeak(idx);
    m_fValue = fValue;

    QTime currentTime = QTime::currentTime();
    int msecsElapsed = m_lastUpdate.msecsTo(currentTime);
    m_lastUpdate = currentTime;
    updateState(msecsElapsed);

    //Post a paintEvent() message, so that the widget repaints.
    update();
}

void WVuMeter::setPeak(int pos)
{
    if(pos > m_iPeakPos)
    {
        m_iPeakPos = pos;
        m_iPeakHoldCountdown = m_iPeakHoldTime;
    }
}

void WVuMeter::updateState(int msecsElapsed)
{
    // If we're holding at a peak then don't update anything
    m_iPeakHoldCountdown -= msecsElapsed;
    if (m_iPeakHoldCountdown > 0) {
        return;
    } else {
        m_iPeakHoldCountdown = 0;
    }

    // Otherwise, decrement the peak position by the fall step size times the
    // milliseconds elapsed over the fall time multiplier. The peak will fall
    // FallStep times (out of 128 steps) every FallTime milliseconds.

    m_iPeakPos -= float(m_iPeakFallStep) * float(msecsElapsed) / float(m_iPeakFallTime);
    if (m_iPeakPos < 0)
        m_iPeakPos = 0;
}

void WVuMeter::paintEvent(QPaintEvent *)
{
    ScopedTimer t("WVuMeter::paintEvent");
    if (m_pPixmapBack!=0)
    {
        int idx = (int)(m_fValue*(float)(m_iNoPos)/128.);

        // Range check
        if (idx>m_iNoPos)
            idx = m_iNoPos;
        else if (idx<0)
            idx = 0;

        QPainter painter(this);
        // Draw back
        painter.drawPixmap(0, 0, *m_pPixmapBack);

        // Draw (part of) vu
        if (m_bHorizontal)
        {
            //This is a hack to fix something weird with horizontal VU meters:
            if(idx == 0)
                idx = 1;
            painter.drawPixmap(0, 0, *m_pPixmapVu, 0, 0, idx, m_pPixmapVu->height());
            if(m_iPeakHoldSize > 0 && m_iPeakPos > 0)
            {
                painter.drawPixmap(m_iPeakPos-m_iPeakHoldSize, 0, *m_pPixmapVu,
                    m_iPeakPos-m_iPeakHoldSize, 0, m_iPeakHoldSize, m_pPixmapVu->height());
            }
        }
        else
        {
            painter.drawPixmap(0, m_iNoPos-idx, *m_pPixmapVu, 0, m_iNoPos-idx, m_pPixmapVu->width(), idx);
            if(m_iPeakHoldSize > 0 && m_iPeakPos > 0)
            {
                painter.drawPixmap(0, m_pPixmapVu->height()-m_iPeakPos, *m_pPixmapVu,
                    0, m_pPixmapVu->height()-m_iPeakPos, m_pPixmapVu->width(), m_iPeakHoldSize);
            }
        }
    }
}



