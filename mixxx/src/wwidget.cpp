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

#include "wwidget.h"

WWidget::WWidget(QWidget *parent, const char *name, WFlags flags) : QWidget(parent,name, flags|WStaticContents|WRepaintNoErase|WResizeNoErase)
{
    m_fValue = 0.;
    connect(this, SIGNAL(valueChangedLeftDown(double)), this, SLOT(slotReEmitValueDown(double)));
    connect(this, SIGNAL(valueChangedRightDown(double)), this, SLOT(slotReEmitValueDown(double)));
    connect(this, SIGNAL(valueChangedLeftUp(double)), this, SLOT(slotReEmitValueUp(double)));
    connect(this, SIGNAL(valueChangedRightUp(double)), this, SLOT(slotReEmitValueUp(double)));

    setBackgroundMode(Qt::NoBackground);
}

WWidget::~WWidget()
{
}

void WWidget::setValue(double fValue)
{
    m_fValue = fValue;
    update();
}

void WWidget::slotReEmitValueDown(double fValue)
{
    emit(valueChangedDown(fValue));
}

void WWidget::slotReEmitValueUp(double fValue)
{
    emit(valueChangedUp(fValue));
}
