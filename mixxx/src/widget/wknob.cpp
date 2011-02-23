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
#include "wpixmapstore.h"

#include <QPixmap>
#include <QtDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>

WKnob::WKnob(QWidget * parent, float defaultValue)
    : WAbstractControl(parent, defaultValue)
{
    m_pPixmaps = 0;
    m_pPixmapBack = 0;
    m_bDisabledLoaded = false;
    setPositions(0);
}

WKnob::~WKnob()
{
    resetPositions();
}

void WKnob::setup(QDomNode node)
{
    // Set background pixmap if available
    if (!selectNode(node, "BackPath").isNull())
        setPixmapBackground(getPath(selectNodeQString(node, "BackPath")));

    // Number of states. Depends if disabled pics are defined as well
    if (!selectNode(node, "DisabledPath").isNull())
        setPositions(selectNodeInt(node, "NumberStates"),true);
    else
        setPositions(selectNodeInt(node, "NumberStates"),false);

    // Load knob pixmaps
    QString path = selectNodeQString(node, "Path");
    for (int i=0; i<m_iNoPos; ++i)
        setPixmap(i, getPath(path.arg(i)));

    // See if disabled images is defined, and load them...
    if (!selectNode(node, "DisabledPath").isNull())
    {
        path = selectNodeQString(node, "DisabledPath");
        for (int i=0; i<m_iNoPos; ++i)
            setPixmap(i+m_iNoPos, getPath(path.arg(i)));
        m_bDisabledLoaded = true;
    }
}

void WKnob::setPositions(int iNoPos, bool bIncludingDisabled)
{
    m_iNoPos = iNoPos;
    m_iPos = 0;

    resetPositions();

    if (m_iNoPos>0)
    {
        int pics = m_iNoPos;
        if (bIncludingDisabled)
            pics *= 2;

        m_pPixmaps = new QPixmap*[pics];
        for (int i=0; i<pics; i++)
            m_pPixmaps[i] = 0;
    }
}

void WKnob::resetPositions()
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

void WKnob::setPixmap(int iPos, const QString &filename)
{
    m_pPixmaps[iPos] = WPixmapStore::getPixmap(filename);
    if (!m_pPixmaps[iPos])
        qDebug() << "WKnob: Error loading pixmap" << filename;
    setFixedSize(m_pPixmaps[iPos]->size());
}

void WKnob::setPixmapBackground(const QString &filename)
{
    // Load background pixmap
    m_pPixmapBack = WPixmapStore::getPixmap(filename);
    if (!m_pPixmapBack)
        qDebug() << "WKnob: Error loading background pixmap:" << filename;
}

void WKnob::mouseMoveEvent(QMouseEvent * e)
{
    if (!m_bRightButtonPressed) {
        m_fValue += (m_dStartValue-e->y());
        m_dStartValue = e->y();
        if (m_fValue>127.)
            m_fValue = 127.;
        else if (m_fValue<0.)
            m_fValue = 0.;
        emit(valueChangedLeftDown(m_fValue));
        update();
    }
}

void WKnob::mousePressEvent(QMouseEvent * e)
{
    m_dStartValue = e->y();

    if (e->button() == Qt::RightButton)
    {
        reset();
        m_bRightButtonPressed = true;
    }
}

void WKnob::mouseReleaseEvent(QMouseEvent * e)
{
    if (e->button()==Qt::LeftButton)
        emit(valueChangedLeftUp(m_fValue));
    else if (e->button()==Qt::RightButton)
        m_bRightButtonPressed = false;
        //emit(valueChangedRightUp(m_fValue));

    update();
}

void WKnob::wheelEvent(QWheelEvent *e)
{
    double wheelDirection = ((QWheelEvent *)e)->delta() / 120.;
    double newValue = getValue() + (wheelDirection);
    this->updateValue(newValue);

    e->accept();

    //e->ignore();
}

void WKnob::paintEvent(QPaintEvent *)
{
    if (m_pPixmaps>0)
    {
        int idx = (int)(((m_fValue-64.)*(((float)m_iNoPos-1.)/127.))+((float)m_iNoPos/2.));
        // Range check
        if (idx>(m_iNoPos-1))
            idx = m_iNoPos-1;
        else if (idx<0)
            idx = 0;

        // Disabled pixmaps are placed ahead of normal pixmaps in the buffer. Use them
        // if the widget is disabled and the disabled pixmaps are loaded.
        if (m_bOff && m_bDisabledLoaded)
            idx += m_iNoPos;

        QPainter p(this);
        // Paint background
        //p.drawPixmap(0, 0, m_pPixmapBack);
        // Paint button
        p.drawPixmap(0, 0, *m_pPixmaps[idx]);
    }
}
