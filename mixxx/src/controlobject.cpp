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
#include "controleventmidi.h"
#include "controleventengine.h"
#include "midiobject.h"


// Static member variable definition
ConfigObject<ConfigValueMidi> *ControlObject::config = 0;
QPtrQueue<ControlQueueEngineItem> ControlObject::queue;
QMutex ControlObject::queueMutex;
QPtrList<ControlObject> ControlObject::list;
QWidget *ControlObject::spParentWidget = 0;


ControlObject::ControlObject()
{
    value = 0.;
    installEventFilter(this);
    m_pControlEngine = 0;
    m_pAccel = 0;
}

ControlObject::ControlObject(ConfigKey key)
{
    m_pControlEngine = 0;
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

bool ControlObject::connectControls(ConfigKey src, ConfigKey dest)
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

void ControlObject::setControlEngine(ControlEngine *pControlEngine)
{
    m_pControlEngine = pControlEngine;
}

void ControlObject::setWidget(QWidget *widget, ConfigKey key, bool emitOnDownPress, Qt::ButtonState state)
{
    // Loop through the list of ConfigObjects to find one matching
    // key, and associate the found object with the widget.
    ControlObject *c;
    for (c=list.first(); c; c=list.next())
    {
        if (c->cfgOption->key->group == key.group && c->cfgOption->key->item == key.item)
        {
            c->setWidget(widget, emitOnDownPress, state);
            break;
        }
    }
}

void ControlObject::setWidget(QWidget *widget, bool emitOnDownPress, Qt::ButtonState state)
{
    if (emitOnDownPress)
    {
        if (state == Qt::NoButton)
            QApplication::connect(widget, SIGNAL(valueChangedDown(float)), this,   SLOT(slotSetPositionExtern(float)));
        else if (state == Qt::LeftButton)
            QApplication::connect(widget, SIGNAL(valueChangedLeftDown(float)), this,   SLOT(slotSetPositionExtern(float)));
        else if (state == Qt::RightButton)
            QApplication::connect(widget, SIGNAL(valueChangedRightDown(float)), this,   SLOT(slotSetPositionExtern(float)));
    }
    else
    {
        if (state == Qt::NoButton)
            QApplication::connect(widget, SIGNAL(valueChangedUp(float)), this,   SLOT(slotSetPositionExtern(float)));
        else if (state == Qt::LeftButton)
            QApplication::connect(widget, SIGNAL(valueChangedLeftUp(float)), this,   SLOT(slotSetPositionExtern(float)));
        else if (state == Qt::RightButton)
            QApplication::connect(widget, SIGNAL(valueChangedRightUp(float)), this,   SLOT(slotSetPositionExtern(float)));
    }
    
    QApplication::connect(this,   SIGNAL(updateGUI(float)),    widget, SLOT(setValue(float)));

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
    if (m_pControlEngine!=0)
    {
        ControlQueueEngineItem *item = new ControlQueueEngineItem;
        item->ptr = m_pControlEngine;
        item->value = value;

        queueMutex.lock();
        queue.enqueue(item);
        queueMutex.unlock();
    }
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

void ControlObject::sync()
{
    // If possible lock mutex and process queue
    if (queueMutex.tryLock())
    {
        //qDebug("queue len %i",queue.count());

        ControlQueueEngineItem *item = queue.dequeue();
        while (item!=0)
        {
            item->ptr->setExtern(item->value);
            delete item;
            item = queue.dequeue();
        }

        // Unlock mutex
        queueMutex.unlock();
    }
}
 
