/***************************************************************************
                          controleventengine.cpp  -  description
                             -------------------
    begin                : Thu Feb 20 2003
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

#include "controleventengine.h"

ControlEventEngine::ControlEventEngine(float value) : QCustomEvent(10000), v(value)
{
};

ControlEventEngine::~ControlEventEngine()
{
}

float ControlEventEngine::value() const
{
    return v;
};
