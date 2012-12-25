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

#include <QtDebug>
#include <QHash>
#include <QSet>
#include <QMutexLocker>

#include "controlobject.h"
#include "controlevent.h"
#include "util/stat.h"
#include "util/timer.h"

// Static member variable definition
QHash<ConfigKey,ControlObject*> ControlObject::m_sqCOHash;
QMutex ControlObject::m_sqCOHashMutex;

QMutex ControlObject::m_sqQueueMutexMidi;
QMutex ControlObject::m_sqQueueMutexThread;
QQueue<QueueObjectMidi*> ControlObject::m_sqQueueMidi;
QQueue<QueueObjectThread*> ControlObject::m_sqQueueThread;
QAtomicPointer<ControlObject> ControlObject::s_changedControls;

ControlObject::ControlObject()
        : m_dValue(0),
          m_dDefaultValue(0),
          m_bIgnoreNops(true),
          m_changedControlsNext(NULL) {
}

ControlObject::ControlObject(ConfigKey key, bool bIgnoreNops, bool track)
        : m_dValue(0),
          m_dDefaultValue(0),
          m_key(key),
          m_bIgnoreNops(bIgnoreNops),
          m_bTrack(track),
          m_trackKey("control " + m_key.group + "," + m_key.item),
          m_trackType(Stat::UNSPECIFIED),
          m_trackFlags(Stat::COUNT | Stat::SUM | Stat::AVERAGE |
                       Stat::SAMPLE_VARIANCE | Stat::MIN | Stat::MAX),
          m_changedControlsNext(NULL) {
    m_sqCOHashMutex.lock();
    m_sqCOHash.insert(m_key, this);
    m_sqCOHashMutex.unlock();

    if (m_bTrack) {
        // TODO(rryan): Make configurable.
        Stat::track(m_trackKey, static_cast<Stat::StatType>(m_trackType),
                    static_cast<Stat::ComputeFlags>(m_trackFlags), m_dValue);
    }
}

ControlObject::ControlObject(const QString& group, const QString& item, bool bIgnoreNops)
        : m_dValue(0),
          m_dDefaultValue(0),
          m_key(group, item),
          m_bIgnoreNops(bIgnoreNops),
          m_changedControlsNext(NULL) {
    m_sqCOHashMutex.lock();
    m_sqCOHash.insert(m_key, this);
    m_sqCOHashMutex.unlock();
}

ControlObject::~ControlObject() {
    m_sqCOHashMutex.lock();
    m_sqCOHash.remove(m_key);
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
    if (m_changedControlsNext || s_changedControls == this) {
        qDebug() << "Deleting" << m_key.group << m_key.item << "still in queue. Syncing.";
        ControlObject::sync();
    }
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
    // qDebug() << "updateProxies: Group" << m_key.group << "/ Item" << m_key.item;
    m_qProxyListMutex.lock();
    QList<ControlObjectThread*> proxyList = m_qProxyList;
    m_qProxyListMutex.unlock();

    QListIterator<ControlObjectThread*> it(proxyList);
    while (it.hasNext())
    {
        obj = it.next();
        if (obj!=pProxyNoUpdate)
        {
            // qDebug() << "upd" << this->getKey().item;
            bUpdateSuccess = bUpdateSuccess && obj->setExtern(m_dValue);
        }
    }
    return bUpdateSuccess;
}

void ControlObject::getControls(QList<ControlObject*>* pControlList) {
    m_sqCOHashMutex.lock();
    for (QHash<ConfigKey, ControlObject*>::const_iterator it = m_sqCOHash.begin();
         it != m_sqCOHash.end(); ++it) {
        pControlList->push_back(it.value());
    }
    m_sqCOHashMutex.unlock();
}

ControlObject* ControlObject::getControl(const ConfigKey& key) {
    //qDebug() << "ControlObject::getControl for (" << key.group << "," << key.item << ")";
    QMutexLocker locker(&m_sqCOHashMutex);
    QHash<ConfigKey, ControlObject*>::const_iterator it = m_sqCOHash.find(key);
    if (it != m_sqCOHash.end()) {
        ControlObject* co = it.value();
        return co;
    }
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
    if (m_bTrack) {
        Stat::track(m_trackKey, static_cast<Stat::StatType>(m_trackType),
                    static_cast<Stat::ComputeFlags>(m_trackFlags), m_dValue);
    }
    emit(valueChangedFromEngine(m_dValue));
}

void ControlObject::setValueFromMidi(MidiOpCode o, double v)
{
    Q_UNUSED(o);
    m_dValue = v;
    if (m_bTrack) {
        Stat::track(m_trackKey, static_cast<Stat::StatType>(m_trackType),
                    static_cast<Stat::ComputeFlags>(m_trackFlags), m_dValue);
    }
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
    if (m_bTrack) {
        Stat::track(m_trackKey, static_cast<Stat::StatType>(m_trackType),
                    static_cast<Stat::ComputeFlags>(m_trackFlags), m_dValue);
    }
    emit(valueChanged(m_dValue));
}

void ControlObject::addToChangedList() {
    // Already in the list.
    if (s_changedControls == this || m_changedControlsNext != NULL) {
        return;
    }

    while (true) {
        m_changedControlsNext = s_changedControls;
        // Try to switch ourselves to be the new head as long as what we just
        // read as the head is still the head.
        if (s_changedControls.testAndSetOrdered(m_changedControlsNext, this)) {
            // We are the new head.
            return;
        }
    }
}


void ControlObject::set(double dValue)
{
    if (m_bIgnoreNops && m_dValue == dValue)
        return;

    setValueFromEngine(dValue);
    addToChangedList();
}

void ControlObject::add(double dValue)
{
    if (m_bIgnoreNops && !dValue)
        return;

    setValueFromEngine(m_dValue+dValue);
    addToChangedList();
}

void ControlObject::sub(double dValue)
{
    if (m_bIgnoreNops && !dValue)
        return;

    setValueFromEngine(m_dValue-dValue);
    addToChangedList();
}

double ControlObject::getValueFromWidget(double v)
{
    return v;
}

double ControlObject::getValueToWidget(double v)
{
    return v;
}

void ControlObject::sync() {
    // Update control objects with values recieved from threads. We tryLock
    // because ControlObject::sync() is re-entrant (even though we just run
    // sync() in the main loop). A slot invoked by sync() can create a modal
    // dialog which effectively blocks sync() but continues spinning the Qt
    // event loop. When sync() runs again, it is re-entrant if the modal dialog
    // is still up.
    if (m_sqQueueMutexThread.tryLock()) {

        // We have to make a copy of the queue otherwise we can get deadlocks
        // since responding to a queued event via setValueFromThread can trigger
        // a slot which in turn could cause a lock of m_sqQueueMutexThread.
        QQueue<QueueObjectThread*> qQueueThread = m_sqQueueThread;
        m_sqQueueThread.clear();
        m_sqQueueMutexThread.unlock();

        while (!qQueueThread.isEmpty()) {
            QueueObjectThread* obj = qQueueThread.dequeue();
            if (obj == NULL) {
                continue;
            }
            if (obj->pControlObject) {
                obj->pControlObject->setValueFromThread(obj->value);
                obj->pControlObject->updateProxies(obj->pControlObjectThread);
            }
            delete obj;
        }
    }

    // Update control objects with values recieved from MIDI. We tryLock because
    // ControlObject::sync() is re-entrant (even though we just run sync() in
    // the main loop). A slot invoked by sync() can create a modal dialog which
    // effectively blocks sync() but continues spinning the Qt event loop. When
    // sync() runs again, it is re-entrant if the modal dialog is still up.
    if (m_sqQueueMutexMidi.tryLock()) {
        // We have to make a copy of the queue otherwise we can get deadlocks
        // since responding to a queued event via setValueFromMidi can trigger a
        // slot which in turn could cause a lock of m_sqQueueMutexMidi.
        QQueue<QueueObjectMidi*> qQueueMidi = m_sqQueueMidi;
        m_sqQueueMidi.clear();
        m_sqQueueMutexMidi.unlock();

        while (!qQueueMidi.isEmpty()) {
            QueueObjectMidi* obj = qQueueMidi.dequeue();
            if (obj == NULL) {
                continue;
            }
            if (obj->pControlObject) {
                obj->pControlObject->setValueFromMidi(obj->opcode, obj->value);
                obj->pControlObject->updateProxies(NULL);
            }
            delete obj;
        }
    }

    // Update app threads (ControlObjectThread derived objects) with changes in
    // the corresponding ControlObjects. These updates should only occour if no
    // changes has been in the object from widgets, midi or application threads.
    {
        ScopedTimer t("ControlObject::sync qQueueChanges");

        while (true) {
            ControlObject* obj = s_changedControls;
            if (obj == NULL) {
                break;
            }
            if (s_changedControls.testAndSetOrdered(obj, obj->m_changedControlsNext)) {
                obj->m_changedControlsNext = NULL;
                // If update is not successful, enqueue again
                if (!obj->updateProxies()) {
                    obj->addToChangedList();
                    Stat::track("ControlObject::sync qQueueChanges failed CO update",
                                Stat::UNSPECIFIED, Stat::COUNT | Stat::SUM | Stat::AVERAGE, 1.0);
                }
            }
        }
    }
}
