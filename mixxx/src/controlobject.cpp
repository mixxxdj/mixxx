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
QWidget *ControlObject::spParentWidget = 0;


ControlObject::ControlObject()
{
    value = 0.;
    installEventFilter(this);
    m_pAccel = 0;
}

ControlObject::ControlObject(ConfigKey key)
{
    controlEngineNo = -1;
    installEventFilter(this);

    // Retreive configuration option object
    cfgOption = config->get(key);

    list.append(this);
    m_pAccel = 0;
}

ControlObject::~ControlObject()
{
    list.remove(this);
}

bool ControlObject::connect(ConfigKey src, ConfigKey dest)
{
qDebug("unfinished");
    // Find src object
    ControlObject *pSrc = 0;
    for (pSrc=list.first(); pSrc; pSrc=list.next())
    {
        qDebug("(%s,%s) (%s,%s)",pSrc->cfgOption->key->group.latin1(),src.group.latin1(),pSrc->cfgOption->key->item.latin1(),src.item.latin1());
        if ((pSrc->cfgOption->key->group == src.group) && (pSrc->cfgOption->key->item == src.item))
            break;
    }
    if (pSrc==0)
        return false;
        
    // Find dest object
    ControlObject *pDest = 0;
    for (pDest=list.first(); pDest; pDest=list.next())
    {
        if ((pDest->cfgOption->key->group == dest.group) && (pDest->cfgOption->key->item == dest.item))
            break;
    }
    if (pDest==0)
        return false;

    QApplication::connect(pSrc, SIGNAL(valueChanged(FLOAT_TYPE)), pDest, SLOT(setValue(FLOAT_TYPE)));

    return true;
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

void ControlObject::setWidget(QWidget *widget)
{
    QApplication::connect(widget, SIGNAL(valueChanged(int)), this,   SLOT(slotSetPosition(int)));
    QApplication::connect(this,   SIGNAL(updateGUI(int)),    widget, SLOT(setValue(int)));

    forceGUIUpdate();
}

void ControlObject::setValue(FLOAT_TYPE v)
{
    //qDebug("thread id: %p",pthread_self());

    value = v;
    forceGUIUpdate();
}

FLOAT_TYPE ControlObject::getValue()
{
    //qDebug("thread id: %p",pthread_self());

    return value;
}

void ControlObject::setParentWidget(QWidget *pParentWidget)
{
    spParentWidget = pParentWidget;
}

QWidget *ControlObject::getParentWidget()
{
    return spParentWidget;
}

void ControlObject::emitValueChanged(FLOAT_TYPE value)
{
    //qDebug("thread id: %p",pthread_self());

    if (controlEngineNo>-1)
    {
        ControlEngineQueueItem *item = new ControlEngineQueueItem;
        item->no = controlEngineNo;
        item->value = value;
        if (queue)
        {
            queue->add(item);
//            qDebug("no %i, add %f",item->no,value);
        }
    }
    //qDebug("val %f",value);

// DO SOMETHING HERE *********************** ADD TO QUEUE
//    if (controlEngine>0)
//        controlEngine->setExtern(value);

    emit(valueChanged(value));
}

/** Called when a midi event is received from MidiObject */
void ControlObject::midi(MidiCategory category, char channel, char control, char value)
{
//    qDebug("Received midi message: ch %i no %i val %i",(int)channel,(int)control,(int)value);

    // Check the potmeters:
    ControlObject *c;
    for (c=list.first(); c; c=list.next())
    {
        if ((c->cfgOption->val->midino == control) &
            (c->cfgOption->val->midichannel == channel))
        {
            c->slotSetPositionMidi(category, (int)value); // 127-value
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
        midi(cem->category(), cem->channel(), cem->control(), cem->value());
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

/*
void ControlObject::setAccelUp(const QKeySequence key)
{
    qDebug("Cannot call setAccelUp for %s", print());
}

void ControlObject::setAccelDown(const QKeySequence key)
{
    qDebug("Cannot call setAccelDown for %s", print());
}

void ControlObject::forceGUIUpdate()
{
    qDebug("Cannot call forceGUIUpdate() for %s", print());
}

void ControlObject::slotSetPosition(int dummy)
{
    qDebug("Cannot call slotSetPosition() for %s", print());
}
    
void ControlObject::slotSetPositionMidi(MidiCategory c, int v)
{
    qDebug("Cannot call slotSetPositionMidi() for %s", print());
}
*/ 
