/***************************************************************************
                          controlevent.cpp  -  description
                             -------------------
    begin                : Mon Sep 27 2004
    copyright            : (C) 2004 by Tue Haste Andersen
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

#include "controlevent.h"
//Added by qt3to4:
#include <QCustomEvent>

ControlEvent::ControlEvent(double dValue) : QCustomEvent(10000), m_dValue(dValue)
{
}

ControlEvent::~ControlEvent()
{
}

double ControlEvent::value() const
{
    return m_dValue;
}
