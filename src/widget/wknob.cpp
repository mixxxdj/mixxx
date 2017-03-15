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

#include <QtDebug>
#include <QMouseEvent>
#include <QWheelEvent>

#include "widget/wknob.h"

WKnob::WKnob(QWidget* pParent)
        : WDisplay(pParent) {
}

void WKnob::mouseMoveEvent(QMouseEvent* e) {
    m_handler.mouseMoveEvent(this, e);
}

void WKnob::mousePressEvent(QMouseEvent* e) {
    m_handler.mousePressEvent(this, e);
}

void WKnob::mouseReleaseEvent(QMouseEvent* e) {
    m_handler.mouseReleaseEvent(this, e);
}

void WKnob::wheelEvent(QWheelEvent* e) {
    m_handler.wheelEvent(this, e);
}
