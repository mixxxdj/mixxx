/***************************************************************************
                 wabstractcontrol.cpp  -  Abstract Control Widget
                             -------------------
    copyright            : (C) 2007 by Wesley Stessens
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "wabstractcontrol.h"

WAbstractControl::WAbstractControl(QWidget *parent, const char *name, float defaultValue)
    : WWidget(parent,name)
{
    m_fDefaultValue = defaultValue;
    m_bRightButtonPressed = false;
}

WAbstractControl::~WAbstractControl()
{
}

void WAbstractControl::setDefaultValue(float value)
{
    m_fDefaultValue = value;
}

void WAbstractControl::reset()
{
    setValue(m_fDefaultValue);
    emit(valueChangedLeftUp(m_fValue));
    emit(valueChangedLeftDown(m_fValue));
}
