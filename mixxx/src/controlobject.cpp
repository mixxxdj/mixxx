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
#include <qthread.h>

// Static member variable definition
ConfigObject<ConfigValueMidi> *ControlObject::config = 0;
MidiObject *ControlObject::midi = 0;

ControlObject::ControlObject()
{    installEventFilter(this);
}

ControlObject::ControlObject(ConfigKey key)
{
    installEventFilter(this);

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
    s->append(cfgOption->key->item.ascii());
    return s;
}

void ControlObject::slotSetPositionMidi(int pos)
{
    // IF updateGUI is emitted directly from another thread than the main thread
    // (in this case the midi thread) the gui widgets will be updated from a non-
    // main thread which is BAD. To avoid this, an event is posted from and to the
    // same control object. When the event is received the updateGUI signal is
    // emitted. The event dispatching, sees to that the event is recieved in the
    // main thread instead of the sending thread.
    tmpPos = pos;
    QThread::postEvent(this,new QEvent((QEvent::Type)1002));
}

bool ControlObject::eventFilter(QObject *o, QEvent *e)
{
    // If a user event is received, emit an updateGUI signal
    // and force screen update
    if (e->type() == (QEvent::Type)1002)
    {
        emit(updateGUI(tmpPos));
    }
    else
        // Standard event processing
        return QObject::eventFilter(o,e);

    return TRUE;
}
