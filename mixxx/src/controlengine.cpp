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



QPtrList<ControlEngine> *ControlEngine::list = new QPtrList<ControlEngine>;

ControlEngine::ControlEngine(ControlObject *_controlObject)
{
    controlObject = _controlObject;

    notifyobj = 0;
    
    // Insert this object into the static list of controlEngines
    list->append(this);
    
    // Update associated controlObject with the number of this controlEngine in the static list
    controlObject->setControlEngine(list->count()-1);

    value = controlObject->getValue();
}

QPtrList<ControlEngine> *ControlEngine::getList()
{
    return list;
}

ControlEngine::~ControlEngine()
{
    delete controlObject;
}

void ControlEngine::setNotify(EngineObject *_notifyobj, void (EngineObject::*_notifymethod)(double))
{
    notifyobj = _notifyobj;
    notifymethod = _notifymethod;
}

double ControlEngine::get()
{
    return value;
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
    if (notifyobj!=0)
        (notifyobj->*notifymethod)(v);
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
