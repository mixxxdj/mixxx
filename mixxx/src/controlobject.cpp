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
#include <QtDebug>
#include <QHash>
#include "controlobject.h"
#include "controlevent.h"

// Static member variable definition
QHash<ConfigKey,ControlObject*> ControlObject::m_sqCOHash;
QMutex ControlObject::m_sqCOHashMutex;

QMutex ControlObject::m_sqQueueMutexMidi;
QMutex ControlObject::m_sqQueueMutexThread;
QMutex ControlObject::m_sqQueueMutexChanges;
QQueue<QueueObjectMidi*> ControlObject::m_sqQueueMidi;
QQueue<QueueObjectThread*> ControlObject::m_sqQueueThread;
QQueue<ControlObject*> ControlObject::m_sqQueueChanges;

ControlObject::ControlObject() :
    m_dValue(0),
    m_dDefaultValue(0),
    m_bIgnoreNops(true) {
}

ControlObject::ControlObject(ConfigKey key, bool bIgnoreNops) :
    m_dValue(0),
    m_Key(key),
    m_bIgnoreNops(bIgnoreNops) {
    m_sqCOHashMutex.lock();
    m_sqCOHash.insert(key,this);
    m_sqCOHashMutex.unlock();
}

ControlObject::~ControlObject()
{
    m_sqCOHashMutex.lock();
    m_sqCOHash.remove(m_Key);
    m_sqCOHashMutex.unlock();

    ControlObjectThread * obj;
    m_qProxyListMutex.lock();
    QListIterator<ControlObjectThread*> it(m_qProxyList);
    while (it.hasNext())
    {
        obj = it.next();
        obj->slotParentDead();
    }
    m_qProxyListMutex.unlock();


    m_sqQueueMutexThread.lock();
    QMutableListIterator<QueueObjectThread*> tit(m_sqQueueThread);
    while (tit.hasNext()) {
        QueueObjectThread* tobj = tit.next();
        if (tobj->pControlObject == this) {
            tit.remove();
            delete tobj;
        }
    }
    m_sqQueueMutexThread.unlock();

    m_sqQueueMutexMidi.lock();
    QMutableListIterator<QueueObjectMidi*> mit(m_sqQueueMidi);
    while (mit.hasNext()) {
        QueueObjectMidi* mobj = mit.next();
        if (mobj->pControlObject == this) {
            mit.remove();
            delete mobj;
        }
    }
    m_sqQueueMutexMidi.unlock();

    // Remove this control object from the changes queue, since we're being
    // deleted.
    m_sqQueueMutexChanges.lock();
    m_sqQueueChanges.removeAll(this);
    m_sqQueueMutexChanges.unlock();

}

bool ControlObject::connectControls(ConfigKey src, ConfigKey dest)
{
    // Find src and dest objects
    ControlObject * pSrc = getControl(src);
    ControlObject * pDest = getControl(dest);

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
    ControlObject * pSrc = getControl(key);

    if (pSrc)
    {
        disconnect(pSrc, 0, 0, 0);
        return true;
    }
    else
        return false;
}

void ControlObject::addProxy(ControlObjectThread * pControlObjectThread)
{
    m_qProxyListMutex.lock();
    m_qProxyList.append(pControlObjectThread);
    m_qProxyListMutex.unlock();
}

void ControlObject::removeProxy(ControlObjectThread * pControlObjectThread) {
    m_qProxyListMutex.lock();
    m_qProxyList.removeAll(pControlObjectThread);
    m_qProxyListMutex.unlock();
}

bool ControlObject::updateProxies(ControlObjectThread * pProxyNoUpdate)
{
    ControlObjectThread * obj;
    bool bUpdateSuccess = true;
    // qDebug() << "updateProxies: Group" << m_Key.group << "/ Item" << m_Key.item;
    m_qProxyListMutex.lock();
    QListIterator<ControlObjectThread*> it(m_qProxyList);
    while (it.hasNext())
    {
        obj = it.next();
        if (obj!=pProxyNoUpdate)
        {
            // qDebug() << "upd" << this->getKey().item;
            bUpdateSuccess = bUpdateSuccess && obj->setExtern(m_dValue);
        }
    }
    m_qProxyListMutex.unlock();
    return bUpdateSuccess;
}

void ControlObject::getControls(QList<ControlObject*>* pControlList) {
    m_sqCOHashMutex.lock();
    for (QHash<ConfigKey, ControlObject*>::const_iterator it = m_sqCOHash.constBegin();
         it != m_sqCOHash.constEnd(); ++it) {
        pControlList->push_back(it.value());
    }
    m_sqCOHashMutex.unlock();
}

ControlObject * ControlObject::getControl(ConfigKey key)
{
    //qDebug() << "ControlObject::getControl for (" << key.group << "," << key.item << ")";
    m_sqCOHashMutex.lock();
    if(m_sqCOHash.contains(key)) {
        ControlObject *co = m_sqCOHash[key];
        m_sqCOHashMutex.unlock();
        return co;
    }
    m_sqCOHashMutex.unlock();

    qWarning() << "ControlObject::getControl returning NULL for (" << key.group << "," << key.item << ")";
    return NULL;
}

void ControlObject::queueFromThread(double dValue, ControlObjectThread * pControlObjectThread)
{
    QueueObjectThread * p = new QueueObjectThread;
    p->pControlObjectThread = pControlObjectThread;
    p->pControlObject = this;
    p->value = dValue;

    m_sqQueueMutexThread.lock();
    m_sqQueueThread.enqueue(p);
    m_sqQueueMutexThread.unlock();
}

void ControlObject::queueFromMidi(MidiOpCode o, double v)
{
    QueueObjectMidi * p = new QueueObjectMidi;
    p->pControlObject = this;
    p->opcode = o;
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

void ControlObject::setValueFromMidi(MidiOpCode o, double v)
{
    Q_UNUSED(o);
    m_dValue = v;
    emit(valueChanged(m_dValue));
}

double ControlObject::GetMidiValue()
{
    return m_dValue;
}

void ControlObject::setValueFromThread(double dValue)
{
    if (m_bIgnoreNops && m_dValue == dValue)
        return;

    m_dValue = dValue;
    emit(valueChanged(m_dValue));
}

void ControlObject::set(double dValue)
{
    if (m_bIgnoreNops && m_dValue == dValue)
        return;

    setValueFromEngine(dValue);
    m_sqQueueMutexChanges.lock();
    m_sqQueueChanges.enqueue(this);
    m_sqQueueMutexChanges.unlock();
}

void ControlObject::add(double dValue)
{
    if (m_bIgnoreNops && !dValue)
        return;

    setValueFromEngine(m_dValue+dValue);
    m_sqQueueMutexChanges.lock();
    m_sqQueueChanges.enqueue(this);
    m_sqQueueMutexChanges.unlock();
}

void ControlObject::sub(double dValue)
{
    if (m_bIgnoreNops && !dValue)
        return;

    setValueFromEngine(m_dValue-dValue);
    m_sqQueueMutexChanges.lock();
    m_sqQueueChanges.enqueue(this);
    m_sqQueueMutexChanges.unlock();
}

double ControlObject::getValueFromWidget(double v)
{
    return v;
}

double ControlObject::getValueToWidget(double v)
{
    return v;
}

void ControlObject::sync()
{
    // Update control objects with values recieved from threads
    if (m_sqQueueMutexThread.tryLock())
    {
        QueueObjectThread * obj;
        while(!m_sqQueueThread.isEmpty())
        {
            obj = m_sqQueueThread.dequeue();

            if (obj->pControlObject)
            {
                obj->pControlObject->setValueFromThread(obj->value);
                obj->pControlObject->updateProxies(obj->pControlObjectThread);
            }
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
        QueueObjectMidi * obj;
        while(!m_sqQueueMidi.isEmpty())
        {
            obj = m_sqQueueMidi.dequeue();
            if (obj == NULL) {
                qDebug() << "Midi sent us a bad object!";
            } else if (obj->pControlObject == NULL) {
                qDebug() << "Midi object with null control object!";
                delete obj;
            } else {
                obj->pControlObject->setValueFromMidi(obj->opcode,obj->value);
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
    ControlObject * obj;
    if(m_sqQueueMutexChanges.tryLock())
    {
        while(!m_sqQueueChanges.isEmpty())
        {
            obj = m_sqQueueChanges.dequeue();

            // If update is not successful, enqueue again
            if (!obj->updateProxies())
                m_sqQueueChanges.enqueue(obj);

        }
        m_sqQueueMutexChanges.unlock();
    }
}
