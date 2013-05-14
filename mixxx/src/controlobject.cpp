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
#include "control.h"
#include "util/stat.h"
#include "util/timer.h"

// Static member variable definition
QHash<ConfigKey,ControlObject*> ControlObject::m_sqCOHash;
QMutex ControlObject::m_sqCOHashMutex;


ControlObject::ControlObject()
        : m_pControl(NULL) {
}

ControlObject::ControlObject(ConfigKey key, bool bIgnoreNops, bool bTrack)
        : m_key(key),
          m_pControl(ControlNumericPrivate::getControl(m_key, true, bIgnoreNops, bTrack)) {
    // TODO(rryan): Set validator on m_pControl.

    connect(m_pControl, SIGNAL(valueChanged(double, QObject*)),
            this, SLOT(privateValueChanged(double, QObject*)));

    m_sqCOHashMutex.lock();
    m_sqCOHash.insert(m_key, this);
    m_sqCOHashMutex.unlock();
}

ControlObject::ControlObject(const QString& group, const QString& item,
                             bool bIgnoreNops, bool bTrack)
        : m_key(group, item),
          m_pControl(ControlNumericPrivate::getControl(m_key, true, bIgnoreNops, bTrack)) {

    connect(m_pControl, SIGNAL(valueChanged(double, QObject*)),
            this, SLOT(privateValueChanged(double, QObject*)));

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
}

void ControlObject::privateValueChanged(double dValue, QObject* pSetter) {
    // Only emit valueChanged() if we did not originate this change.
    if (pSetter != this) {
        emit(valueChanged(dValue));
    }
}

/*
bool ControlObject::connectControls(ConfigKey src, ConfigKey dest)
{
    // Find src and dest objects
    ControlObject * pSrc = getControl(src);
    ControlObject * pDest = getControl(dest);

    if (pSrc && pDest) {
        connect(pSrc, SIGNAL(valueChanged(double)), pDest, SLOT(set(double)));
        return true;
    } else {
        return false;
    }
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
*/

void ControlObject::addProxy(ControlObjectThread * pControlObjectThread) {
    if (m_pControl) {
        connect(m_pControl, SIGNAL(valueChanged(double, QObject*)),
                pControlObjectThread, SLOT(slotValueChanged(double, QObject*)));
    }
    m_qProxyListMutex.lock();
    m_qProxyList.append(pControlObjectThread);
    m_qProxyListMutex.unlock();
}

void ControlObject::removeProxy(ControlObjectThread * pControlObjectThread) {
    if (m_pControl) {
        disconnect(m_pControl, SIGNAL(valueChanged(double, QObject*)),
                   pControlObjectThread, SLOT(slotValueChanged(double, QObject*)));
    }
    m_qProxyListMutex.lock();
    m_qProxyList.removeAll(pControlObjectThread);
    m_qProxyListMutex.unlock();
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

void ControlObject::setValueFromMidi(MidiOpCode o, double v) {
    if (m_pControl) {
        m_pControl->set(v, NULL);
    }
}

double ControlObject::GetMidiValue() {
    return m_pControl ? m_pControl->get() : 0.0;
}

void ControlObject::setValueFromThread(double dValue, QObject* pSender) {
    if (m_pControl) {
        m_pControl->set(dValue, pSender);
    }
}

void ControlObject::add(double dValue) {
    if (m_pControl) {
        m_pControl->add(dValue, this);
    }
}

void ControlObject::sub(double dValue) {
    if (m_pControl) {
        m_pControl->sub(dValue, this);
    }
}

double ControlObject::getValueFromWidget(double v) {
    return v;
}

double ControlObject::getValueToWidget(double v) {
    return v;
}

double ControlObject::get() {
    return m_pControl ? m_pControl->get() : 0.0;
}

void ControlObject::reset() {
    if (m_pControl) {
        m_pControl->reset(this);
    }
}

void ControlObject::set(const double& value) {
    if (m_pControl) {
        m_pControl->set(value, this);
    }
}
