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

#include <qwidget.h>
#include "controlobject.h"
#include "controlevent.h"

// Static member variable definition
QPtrList<ControlObject> ControlObject::m_sqList;
QMutex ControlObject::m_sqQueueMutexMidi;
QMutex ControlObject::m_sqQueueMutexThread;
QPtrQueue<QueueObjectMidi> ControlObject::m_sqQueueMidi;
QPtrQueue<QueueObjectThread> ControlObject::m_sqQueueThread;
QPtrQueue<ControlObject> ControlObject::m_sqQueueChanges;

ControlObject::ControlObject()
{
}

ControlObject::ControlObject(ConfigKey key)
{
    m_dValue = 0.;
    m_Key = key;
    m_sqList.append(this);
}

ControlObject::~ControlObject()
{
    m_sqList.remove(this);
}

bool ControlObject::connectControls(ConfigKey src, ConfigKey dest)
{
    // Find src and dest objects
    ControlObject *pSrc = getControl(src);
    ControlObject *pDest = getControl(dest);

    if (pSrc && pDest)
    {
        connect(pSrc, SIGNAL(valueChanged(double)), pDest, SLOT(set(double)));
        connect(pSrc, SIGNAL(valueChangedFromEngine(double)), pDest, SLOT(set(double)));
        return true;
    }
    else
        return false;
}

bool ControlObject::disconnectControl(ConfigKey key)
{
    // Find src and dest objects
    ControlObject *pSrc = getControl(key);

    if (pSrc)
    {
        disconnect(pSrc, 0, 0, 0);
        return true;
    }
    else
        return false;
}

void ControlObject::addProxy(ControlObjectThread *pControlObjectThread)
{
    m_qProxyList.append(pControlObjectThread);
}

void ControlObject::removeProxy(ControlObjectThread *pControlObjectThread) {
	m_qProxyList.removeRef(pControlObjectThread);
}

bool ControlObject::updateProxies(ControlObjectThread *pProxyNoUpdate)
{
    ControlObjectThread *obj;
    bool bUpdateSuccess = true;
//     qDebug("Key %s,%s",m_Key.group.latin1(), m_Key.item.latin1());
    for (obj = m_qProxyList.first(); obj; obj = m_qProxyList.next())
    {
        if (obj!=pProxyNoUpdate)
        {
//             const char *thisname = this->getKey().item.latin1();
// 	    qDebug("upd %s",thisname);
            bUpdateSuccess = obj->setExtern(m_dValue);
        }
    }
    return bUpdateSuccess;
}

ControlObject *ControlObject::getControl(ConfigKey key)
{
//    qDebug("trying to get group %s, item %s",key.group.latin1(), key.item.latin1());
    
    // Loop through the list of ConfigObjects to find one matching key
    ControlObject *c;
    for (c=m_sqList.first(); c; c=m_sqList.next())
    {
        if (c->getKey().group == key.group && c->getKey().item == key.item)
            return c;
    }
    return 0;
}

void ControlObject::queueFromThread(double dValue, ControlObjectThread *pControlObjectThread)
{
    QueueObjectThread *p = new QueueObjectThread;
    p->pControlObjectThread = pControlObjectThread;
    p->pControlObject = this;
    p->value = dValue;

    m_sqQueueMutexThread.lock();
    m_sqQueueThread.enqueue(p);
    m_sqQueueMutexThread.unlock();
}

void ControlObject::queueFromMidi(MidiCategory c, double v)
{
    QueueObjectMidi *p = new QueueObjectMidi;
    p->pControlObject = this;
    p->category = c;
    p->value = v;

    m_sqQueueMutexMidi.lock();
    m_sqQueueMidi.enqueue(p);
    m_sqQueueMutexMidi.unlock();
}

void ControlObject::setValueFromEngine(double dValue)
{
    m_dValue = dValue;
    emit(valueChangedFromEngine(m_dValue));
}

void ControlObject::setValueFromMidi(MidiCategory, double v)
{
    m_dValue = v;
    emit(valueChanged(m_dValue));
}

double ControlObject::GetMidiValue()
{
    return m_dValue;
}

void ControlObject::setValueFromThread(double dValue)
{
    m_dValue = dValue;
    emit(valueChanged(m_dValue));
}

void ControlObject::set(double dValue)
{
    setValueFromEngine(dValue);
    m_sqQueueChanges.enqueue(this);
}

void ControlObject::add(double dValue)
{
    setValueFromEngine(m_dValue+dValue);
    m_sqQueueChanges.enqueue(this);
}

void ControlObject::sub(double dValue)
{
    setValueFromEngine(m_dValue-dValue);
    m_sqQueueChanges.enqueue(this);
}

double ControlObject::getValueFromWidget(double v)
{
    return v;
}

double ControlObject::getValueToWidget(double v)
{
    return v;
}

ConfigKey ControlObject::getKey()
{
    return m_Key;
}

double ControlObject::get()
{
    return m_dValue;
}

void ControlObject::sync()
{
    // Update control objects with values recieved from threads
    if (m_sqQueueMutexThread.tryLock())
    {
        QueueObjectThread *obj;
        while(!m_sqQueueThread.isEmpty())
        {
            obj = m_sqQueueThread.dequeue();

            obj->pControlObject->setValueFromThread(obj->value);
            obj->pControlObject->updateProxies(obj->pControlObjectThread);
            delete obj;
        }

        //
        // If the object is in m_sqQueueChanges, delete it from that queue.
        //

        m_sqQueueMutexThread.unlock();
    }

    // Update control objects with values recieved from MIDI
    if (m_sqQueueMutexMidi.tryLock())
    {
        QueueObjectMidi *obj;
        while(!m_sqQueueMidi.isEmpty())
        {
            obj = m_sqQueueMidi.dequeue();
	    if (obj == NULL) {
	        qDebug("Midi sent us a bad object!");
	    } else if (obj->pControlObject == NULL) {
		qDebug("Midi object with null control object!");
		delete obj;
	    } else if (obj->category == NULL) {
		qDebug("Midi object with null category!");
		delete obj;
	    /*} else if (obj->value == NULL) {
	        qDebug("Midi object with null value!");
		delete obj;*/
	    } else {
                obj->pControlObject->setValueFromMidi(obj->category, obj->value);
                obj->pControlObject->updateProxies(0);
                delete obj;
	    }
        }

        //
        // If the object is in m_sqQueueChanges, delete it from that queue.
        //

        m_sqQueueMutexMidi.unlock();
    }

    // Update app threads (ControlObjectThread objects) with changes in the corresponding
    // ControlObjects. These updates should only occour if no changes has been in the object
    // from widgets, midi og application threads.
    ControlObject *obj;
    while(!m_sqQueueChanges.isEmpty())
    {
        obj = m_sqQueueChanges.dequeue();

        // If update is not successful, enqueue again
        if (!obj->updateProxies())
            m_sqQueueChanges.enqueue(obj);

    }
}
