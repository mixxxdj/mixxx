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
ConfigObject<ConfigValueMidi> *ControlObject::m_pMidiConfig = 0;
ConfigObject<ConfigValueKbd> *ControlObject::m_pKbdConfig = 0;
QPtrQueue<ControlQueueEngineItem> ControlObject::queue;
QMutex ControlObject::queueMutex;
QPtrList<ControlObject> ControlObject::list;
QWidget *ControlObject::spParentWidget = 0;


ControlObject::ControlObject()
{
    m_dValue = 0.;
    installEventFilter(this);
    m_pControlEngine = 0;
}

ControlObject::ControlObject(ConfigKey key)
{
    m_dValue = 0.;
    m_pControlEngine = 0;
    installEventFilter(this);

    // Retreive midi configuration option object
    m_pMidiConfigOption = m_pMidiConfig->get(key);

    // Retreive keyboard configuration option object
    m_pKbdConfigOption = m_pKbdConfig->get(key);

    list.append(this);
}

ControlObject::~ControlObject()
{
    list.remove(this);
}

bool ControlObject::connectControls(ConfigKey src, ConfigKey dest)
{
    // Find src object
    ControlObject *pSrc = 0;
    for (pSrc=list.first(); pSrc; pSrc=list.next())
    {
//        qDebug("src (%s,%s) (%s,%s)",pSrc->m_pMidiConfigOption->key->group.latin1(),src.group.latin1(),pSrc->m_pMidiConfigOption->key->item.latin1(),src.item.latin1());
        if ((pSrc->m_pMidiConfigOption->key->group == src.group) && (pSrc->m_pMidiConfigOption->key->item == src.item))
            break;
    }
    if (pSrc==0)
        return false;
        
    // Find dest object
    ControlObject *pDest = 0;
    for (pDest=list.first(); pDest; pDest=list.next())
    {
//        qDebug("dest (%s,%s) (%s,%s)",pDest->m_pMidiConfigOption->key->group.latin1(),dest.group.latin1(),pDest->m_pMidiConfigOption->key->item.latin1(),dest.item.latin1());
        if ((pDest->m_pMidiConfigOption->key->group == dest.group) && (pDest->m_pMidiConfigOption->key->item == dest.item))
            break;
    }
    if (pDest==0)
        return false;

    QApplication::connect(pSrc, SIGNAL(signalUpdateApp(double)), pDest, SLOT(setValueFromEngine(double)));

    return true;
}

void ControlObject::setMidiConfig(ConfigObject<ConfigValueMidi> *pMidiConfig)
{
    m_pMidiConfig = pMidiConfig;
}

void ControlObject::setKbdConfig(ConfigObject<ConfigValueKbd> *pKbdConfig)
{
    m_pKbdConfig = pKbdConfig;
}

void ControlObject::setControlEngine(ControlEngine *pControlEngine)
{
    m_pControlEngine = pControlEngine;
}

ControlObject *ControlObject::getControl(ConfigKey key)
{
    // Loop through the list of ConfigObjects to find one matching key
    ControlObject *c;
    for (c=list.first(); c; c=list.next())
    {
        if (c->m_pMidiConfigOption->key->group == key.group && c->m_pMidiConfigOption->key->item == key.item)
            return c;
    }
    return 0;
}

void ControlObject::setWidget(QWidget *widget, ConfigKey key, bool emitOnDownPress, Qt::ButtonState state)
{
    // Loop through the list of ConfigObjects to find one matching
    // key, and associate the found object with the widget.
    ControlObject *c;
    for (c=list.first(); c; c=list.next())
    {
        if (c->m_pMidiConfigOption->key->group == key.group && c->m_pMidiConfigOption->key->item == key.item)
        {
            c->setWidget(widget, emitOnDownPress, state);
            return;
        }
    }
    qDebug("woops, %s",key.item.latin1());
}

void ControlObject::setWidget(QWidget *widget, bool emitOnDownPress, Qt::ButtonState state)
{
    if (emitOnDownPress)
    {
        if (state == Qt::NoButton)
            QApplication::connect(widget, SIGNAL(valueChangedDown(double)), this,   SLOT(setValueFromWidget(double)));
        else if (state == Qt::LeftButton)
            QApplication::connect(widget, SIGNAL(valueChangedLeftDown(double)), this,   SLOT(setValueFromWidget(double)));
        else if (state == Qt::RightButton)
            QApplication::connect(widget, SIGNAL(valueChangedRightDown(double)), this,   SLOT(setValueFromWidget(double)));
    }
    else
    {
        if (state == Qt::NoButton)
            QApplication::connect(widget, SIGNAL(valueChangedUp(double)), this,   SLOT(setValueFromWidget(double)));
        else if (state == Qt::LeftButton)
            QApplication::connect(widget, SIGNAL(valueChangedLeftUp(double)), this,   SLOT(setValueFromWidget(double)));
        else if (state == Qt::RightButton)
            QApplication::connect(widget, SIGNAL(valueChangedRightUp(double)), this,   SLOT(setValueFromWidget(double)));
    }

    QApplication::connect(this,   SIGNAL(signalUpdateWidget(double)),    widget, SLOT(setValue(double)));
    updateWidget();
}

void ControlObject::setWidgetOnOff(QWidget *widget, ConfigKey key)
{
    // Loop through the list of ConfigObjects to find one matching
    // key, and associate the found object with the widget.
    ControlObject *c;
    for (c=list.first(); c; c=list.next())
    {
        if (c->m_pMidiConfigOption->key->group == key.group && c->m_pMidiConfigOption->key->item == key.item)
        {
            c->setWidgetOnOff(widget);
            return;
        }
    }
    qDebug("woops, %s",key.item.latin1());
}

void ControlObject::setWidgetOnOff(QWidget *widget)
{
    QApplication::connect(this,   SIGNAL(signalUpdateWidget(double)),    widget, SLOT(setOnOff(double)));
    updateWidget();
}

void ControlObject::updateFromMidi()
{
    updateEngine();
    updateWidget();
    updateApp();
}

void ControlObject::updateAll()
{
    updateEngine();
    updateWidget();
    updateApp();
}
    
void ControlObject::updateFromEngine()
{
    updateWidget();
    updateApp();
}
    
void ControlObject::updateFromWidget()
{
    updateEngine();
    updateApp();
}

void ControlObject::updateFromApp()
{
    updateEngine();
    updateWidget();
}

void ControlObject::updateEngine()
{
    if (m_pControlEngine!=0)
    {
        ControlQueueEngineItem *item = new ControlQueueEngineItem;
        item->ptr = m_pControlEngine;
        item->value = m_dValue;

        queueMutex.lock();
        queue.enqueue(item);
        queueMutex.unlock();
    }
}

void ControlObject::updateWidget()
{
    emit(signalUpdateWidget(m_dValue));
}

void ControlObject::updateApp()
{
    emit(signalUpdateApp(m_dValue));
}

void ControlObject::setValueFromMidi(MidiCategory, int v)
{
    m_dValue = (double)v;
    updateFromMidi();
}

void ControlObject::setValueFromEngine(double dValue)
{
    m_dValue = dValue;
    updateFromEngine();
}

void ControlObject::setValueFromWidget(double dValue)
{
    m_dValue = dValue;
    updateFromWidget();
}

void ControlObject::setValueFromApp(double dValue)
{
    m_dValue = dValue;
    updateFromApp();
}

double ControlObject::getValue()
{
    return m_dValue;
}

void ControlObject::setParentWidget(QWidget *pParentWidget)
{
    spParentWidget = pParentWidget;
}

QWidget *ControlObject::getParentWidget()
{
    return spParentWidget;
}

void ControlObject::midi(MidiCategory category, char channel, char control, char value)
{
    qDebug("Received midi message: ch %i no %i val %i",(int)channel,(int)control,(int)value);

    // Check the potmeters:
    ControlObject *c;
    for (c=list.first(); c; c=list.next())
    {
        if ((c->m_pMidiConfigOption->val->midino == control) &
            (c->m_pMidiConfigOption->val->midichannel == channel))
        {
            c->setValueFromMidi(category, (int)value);
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
        setValueFromEngine(cee->value());
    }
    else if (e->type() == QEvent::KeyPress)
        qDebug("control Key press");
    else
        // Standard event processing
        return QObject::eventFilter(o,e);

    return TRUE;
}

bool ControlObject::kbdPress(QKeySequence k, bool release)
{
    bool react = false;

    if (!k.isEmpty())
    {
        //qDebug("kbd %s, press %i",((QString)k).latin1(),release);

        // Check the controls...
        ControlObject *c;
        for (c=list.first(); c; c=list.next())
        {
            if (c->m_pKbdConfigOption->val->m_qKey==k)
            {
                if (release)
                    c->setValueFromMidi(NOTE_OFF, 0);
                else
                    c->setValueFromMidi(NOTE_ON, 1);

                //if (release) qDebug("r"); else qDebug("p");

                react = true;

                break;
            }
        }
    }
    return react;
}

void ControlObject::syncControlEngineObjects()
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
 
