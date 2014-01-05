/***************************************************************************
                          wwidget.cpp  -  description
                             -------------------
    begin                : Wed Jun 18 2003
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

#include <QtDebug>

#include "widget/wwidget.h"
#include "controlobject.h"
#include "controlobjectthreadwidget.h"

// Static member variable definition
QString WWidget::m_qPath;

WWidget::WWidget(QWidget* parent, Qt::WindowFlags flags)
        : QWidget(parent, flags),
          m_value(0.0),
          m_bOff(false) {
    connect(this, SIGNAL(valueChangedLeftDown(double)), this, SLOT(slotReEmitValueDown(double)));
    connect(this, SIGNAL(valueChangedRightDown(double)), this, SLOT(slotReEmitValueDown(double)));
    connect(this, SIGNAL(valueChangedLeftUp(double)), this, SLOT(slotReEmitValueUp(double)));
    connect(this, SIGNAL(valueChangedRightUp(double)), this, SLOT(slotReEmitValueUp(double)));

    setAttribute(Qt::WA_StaticContents);
    setFocusPolicy(Qt::ClickFocus);
}

WWidget::~WWidget() {
}

void WWidget::setValue(double value) {
    m_value = value;
    update();
}

void WWidget::setOnOff(double d) {
    if (d == 0.) {
        m_bOff = false;
    } else {
        m_bOff = true;
    }
    repaint();
}

void WWidget::slotReEmitValueDown(double value) {
    emit(valueChangedDown(value));
}

void WWidget::slotReEmitValueUp(double value) {
    emit(valueChangedUp(value));
}

const QString WWidget::getPath(QString location) {
    QString l(location);
    return l.prepend(m_qPath);
}

void WWidget::setPixmapPath(QString qPath) {
    m_qPath = qPath;
}

void WWidget::updateValue(double value) {
    setValue(value);
    emit(valueChangedUp(value));
    emit(valueChangedDown(value));
}
