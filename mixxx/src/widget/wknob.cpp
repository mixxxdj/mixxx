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

#include <QPixmap>
#include <QtDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>

#include "widget/wknob.h"

#include "defs.h"
#include "widget/wpixmapstore.h"

WKnob::WKnob(QWidget * parent)
        : WAbstractControl(parent),
          m_iPos(0),
          m_iNoPos(0),
          m_pPixmaps(NULL),
          m_pPixmapBack(NULL),
          m_bDisabledLoaded(false) {
}

WKnob::~WKnob()
{
    resetPositions();
    if (m_pPixmapBack) {
       WPixmapStore::deletePixmap(m_pPixmapBack);
    }
}

void WKnob::setup(QDomNode node)
{
    // Set background pixmap if available
    if (!selectNode(node, "BackPath").isNull())
        setPixmapBackground(getPath(selectNodeQString(node, "BackPath")));

    // Number of states. Depends if disabled pics are defined as well
    setPositions(selectNodeInt(node, "NumberStates"),
                 !selectNode(node, "DisabledPath").isNull());

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
    resetPositions();

    m_iNoPos = iNoPos;
    m_iPos = 0;

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
        int pics = m_iNoPos;
        if( m_bDisabledLoaded ){
            pics *= 2;
        }
        for (int i=0; i<pics; i++) {
            if (m_pPixmaps[i]) {
                WPixmapStore::deletePixmap(m_pPixmaps[i]);
            }
        }
        delete [] m_pPixmaps;
        m_pPixmaps = NULL;
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
   if (m_pPixmapBack) {
       WPixmapStore::deletePixmap(m_pPixmapBack);
   }
    m_pPixmapBack = WPixmapStore::getPixmap(filename);
    if (!m_pPixmapBack)
        qDebug() << "WKnob: Error loading background pixmap:" << filename;
}

void WKnob::mouseMoveEvent(QMouseEvent * e)
{
    if (!m_bRightButtonPressed) {
        QPoint cur(e->globalPos());
        QPoint diff(cur - m_startPos);
        double dist = sqrt(static_cast<double>(diff.x() * diff.x() + diff.y() * diff.y()));
        bool y_dominant = abs(diff.y()) > abs(diff.x());

        // if y is dominant, then thread an increase in dy as negative (y is
        // pointed downward). Otherwise, if y is not dominant and x has
        // decreased, then thread it as negative.
        if ((y_dominant && diff.y() > 0) || (!y_dominant && diff.x() < 0)) {
            dist = -dist;
        }

        m_fValue += dist;
        QCursor::setPos(m_startPos);

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
    switch (e->button()) {
    case Qt::RightButton:
        emit(valueReset());
        m_bRightButtonPressed = true;
        break;
    case Qt::LeftButton:
    case Qt::MidButton:
        m_startPos = e->globalPos();
        QApplication::setOverrideCursor(Qt::BlankCursor);
        break;
    default:
        break;
    }
}

void WKnob::mouseReleaseEvent(QMouseEvent * e)
{
    switch (e->button()) {
    case Qt::LeftButton:
    case Qt::MidButton:
        QCursor::setPos(m_startPos);
        QApplication::restoreOverrideCursor();
        emit(valueChangedLeftUp(m_fValue));
        break;
    case Qt::RightButton:
        m_bRightButtonPressed = false;
        //emit(valueChangedRightUp(m_fValue));
        break;
    default:
        break;
    }

    update();
}

void WKnob::wheelEvent(QWheelEvent *e)
{
    double wheelDirection = ((QWheelEvent *)e)->delta() / 120.;
    double newValue = getValue() + wheelDirection;

    // Clamp to [0.0, 127.0]
    newValue = math_max(0.0, math_min(127.0, newValue));

    updateValue(newValue);

    e->accept();

    //e->ignore();
}

void WKnob::paintEvent(QPaintEvent *)
{
    if (m_pPixmaps)
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
