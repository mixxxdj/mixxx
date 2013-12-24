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
#include <QApplication>

#include "widget/wknob.h"

#include "defs.h"
#include "widget/wpixmapstore.h"

WKnob::WKnob(QWidget* pParent)
        : WDisplay(pParent),
          m_bRightButtonPressed(false) {
}

WKnob::~WKnob() {
}

void WKnob::mouseMoveEvent(QMouseEvent* e) {
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

        m_value += dist;
        QCursor::setPos(m_startPos);

        if (m_value > 127.0)
            m_value = 127.0;
        else if (m_value < 0.0)
            m_value = 0.0;

        emit(valueChangedLeftDown(m_value));
        update();
    }
}

void WKnob::mousePressEvent(QMouseEvent* e) {
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

void WKnob::mouseReleaseEvent(QMouseEvent* e) {
    switch (e->button()) {
        case Qt::LeftButton:
        case Qt::MidButton:
            QCursor::setPos(m_startPos);
            QApplication::restoreOverrideCursor();
            emit(valueChangedLeftUp(m_value));
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

void WKnob::wheelEvent(QWheelEvent *e) {
    double wheelDirection = e->delta() / 120.;
    double newValue = getValue() + wheelDirection;

    // Clamp to [0.0, 127.0]
    newValue = math_max(0.0, math_min(127.0, newValue));

    updateValue(newValue);

    e->accept();

    //e->ignore();
}

int WKnob::getActivePixmapIndex() const {
    // TODO(rryan): Ew.
    int iNoPos = numPixmaps();
    return (int)(((m_value-64.)*(((float)iNoPos-1.)/127.))+((float)iNoPos/2.));
}
