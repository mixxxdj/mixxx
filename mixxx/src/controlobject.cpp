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
#include "controlengine.h"
#include "controlenginequeue.h"
#include "controleventmidi.h"
#include "controleventengine.h"
#include "midiobject.h"

// Static member variable definition
ConfigObject<ConfigValueMidi> *ControlObject::config = 0;
ControlEngineQueue *ControlObject::queue = 0;
QPtrList<ControlObject> ControlObject::list;

ControlObject::ControlObject(ConfigKey key)
{
    controlEngineNo = -1;
    widget = 0;
    installEventFilter(this);

    // Retreive configuration option object
    cfgOption = config->get(key);

    list.append(this);
}

ControlObject::~ControlObject()
{
    list.remove(this);
}

QString *ControlObject::print()
{
    QString *s = new QString(cfgOption->key->group.ascii());
    s->append(" ");
    s->append(cfgOption->key->item.ascii());
    return s;
}

void ControlObject::setConfig(ConfigObject<ConfigValueMidi> *_config)
{
    config = _config;
}

void ControlObject::setControlEngineQueue(ControlEngineQueue *_queue)
{
    queue = _queue;
}


void ControlObject::setControlEngine(int _controlEngineNo)
{
    controlEngineNo = _controlEngineNo;
}

void ControlObject::setWidget(QWidget *_widget)
{
    widget = _widget;
    connect(widget, SIGNAL(valueChanged(int)), this, SLOT(slotSetPosition(int)));
    connect(this, SIGNAL(updateGUI(int)), widget, SLOT(setValue(int)));

    forceGUIUpdate();
}

void ControlObject::slotSetPositionMidi(int v)
{
    slotSetPosition(v);
    emit(updateGUI(v));
}

void ControlObject::setValue(FLOAT_TYPE v)
{
    value = v;
    forceGUIUpdate();
}

FLOAT_TYPE ControlObject::getValue()
{
    return value;
}

void ControlObject::emitValueChanged(FLOAT_TYPE value)
{
    if (controlEngineNo>-1)
    {
        ControlEngineQueueItem *item = new ControlEngineQueueItem;
        item->no = controlEngineNo;
        item->value = value;
        if (queue)
            queue->add(item);
    }
    //qDebug("val %f",value);

// DO SOMETHING HERE *********************** ADD TO QUEUE
//    if (controlEngine>0)
//        controlEngine->setExtern(value);

    emit(valueChanged(value));
}

/** Called when a midi event is received from MidiObject */
void ControlObject::midi(char channel, char control, char value)
{
    //qDebug("Received midi message: ch %i no %i val %i",(int)channel,(int)midicontrol,(int)midivalue);

    // Check the potmeters:
    ControlObject *c;
    for (c=list.first(); c; c=list.next())
    {
        if ((c->cfgOption->val->midino == control) &
            (c->cfgOption->val->midichannel == channel))
        {
            // Check for possible bit mask
            int midimask = c->cfgOption->val->midimask;

            if (midimask > 0)
                c->slotSetPositionMidi((int)(midimask & value));
            else
                c->slotSetPositionMidi((int)value); // 127-value
            break;
        }
    }
}

bool ControlObject::eventFilter(QObject *o, QEvent *e)
{
    // ControlEventMidi
    if (e->type() == (QEvent::Type)10001)
    {
        ControlEventMidi *cem = (ControlEventMidi *)e;
        midi(cem->channel(), cem->control(), cem->value());
    }
    // ControlEventEngine
    else if (e->type() == (QEvent::Type)10000)
    {
        ControlEventEngine *cee = (ControlEventEngine *)e;
        setValue(cee->value());
    }
    else
        // Standard event processing
        return QObject::eventFilter(o,e);

    return TRUE;
}
