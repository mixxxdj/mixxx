/***************************************************************************
                          controlobject.cpp  -  description
                             -------------------
    begin                : Wed Feb 20 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                : 
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
#include "midiobject.h"

// Static member variable definition
ConfigMIDI *ControlObject::config = 0;
MidiObject *ControlObject::midi = 0;

ControlObject::ControlObject()
{
}

ControlObject::ControlObject(ConfigObject::ConfigKey *key)
{
    // Retreive configuration option object
    cfgOption = config->get(key);

    // Register the control in the midi object:
    midi->add(this);
}

ControlObject::~ControlObject()
{
    midi->remove(this);
}

QString *ControlObject::print()
{
    QString *s = new QString(cfgOption->key->group.ascii());
    s->append(" ");
    s->append(cfgOption->key->control.ascii());
    return s;
}

void ControlObject::slotSetPositionMidi(int pos)
{
    //this->slotSetPosition(pos);
    emit updateGUI(pos);
}









