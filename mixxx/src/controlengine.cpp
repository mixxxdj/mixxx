 /***************************************************************************
                          controlengine.cpp  -  description
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

#include "controlobject.h"
#include "controlengine.h"
#include "controleventengine.h"
#include "engineobject.h"
#include <qapplication.h>

ControlEngine::ControlEngine(ControlObject *_controlObject)
{
    controlObject = _controlObject;

    // Update associated controlObject with the number of this controlEngine in the static list
    controlObject->setControlEngine(this);

    value = controlObject->getValue();
}

ControlEngine::~ControlEngine()
{
    if (controlObject)
        delete controlObject;
}

void ControlEngine::set(double v)
{
    value = v;
 
    ControlEventEngine *e = new ControlEventEngine(v);
    QApplication::postEvent(controlObject,e);
}

void ControlEngine::setExtern(double v)
{
    value = v;
    emit(valueChanged(v));
}

void ControlEngine::add(double v)
{
    value += v;

    ControlEventEngine *e = new ControlEventEngine(v);
    QApplication::postEvent(controlObject,e);
}

void ControlEngine::sub(double v)
{
    value -= v;

    ControlEventEngine *e = new ControlEventEngine(v);
    QApplication::postEvent(controlObject,e);
}
