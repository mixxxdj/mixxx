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
    connect(this, SIGNAL(valueChangedLeftDown(float)), this, SLOT(slotReEmitValueDown(float)));
    connect(this, SIGNAL(valueChangedRightDown(float)), this, SLOT(slotReEmitValueDown(float)));
    connect(this, SIGNAL(valueChangedLeftUp(float)), this, SLOT(slotReEmitValueUp(float)));
    connect(this, SIGNAL(valueChangedRightUp(float)), this, SLOT(slotReEmitValueUp(float)));
}

WWidget::~WWidget()
{
}

void WWidget::setValue(float fValue)
{
    m_fValue = fValue;
    update();
}

void WWidget::slotReEmitValueDown(float fValue)
{
    emit(valueChangedDown(fValue));
}

void WWidget::slotReEmitValueUp(float fValue)
{
    emit(valueChangedUp(fValue));
}
