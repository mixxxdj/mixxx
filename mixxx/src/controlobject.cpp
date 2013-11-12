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
#include "control/control.h"
#include "util/stat.h"
#include "util/timer.h"

// Static member variable definition
QHash<ConfigKey,ControlObject*> ControlObject::m_sqCOHash;
QMutex ControlObject::m_sqCOHashMutex;


ControlObject::ControlObject()
        : m_pControl(NULL) {
}

ControlObject::ControlObject(ConfigKey key, bool bIgnoreNops, bool bTrack)
        : m_pControl(NULL) {
    initialize(key, bIgnoreNops, bTrack);
}

ControlObject::ControlObject(const QString& group, const QString& item,
                             bool bIgnoreNops, bool bTrack)
        : m_pControl(NULL) {
    initialize(ConfigKey(group, item), bIgnoreNops, bTrack);
}

ControlObject::~ControlObject() {
    m_sqCOHashMutex.lock();
    m_sqCOHash.remove(m_key);
    m_sqCOHashMutex.unlock();
}

void ControlObject::initialize(ConfigKey key, bool bIgnoreNops, bool bTrack) {
    m_key = key;
    m_pControl = ControlDoublePrivate::getControl(m_key, true, bIgnoreNops, bTrack);
    connect(m_pControl, SIGNAL(valueChanged(double, QObject*)),
            this, SLOT(privateValueChanged(double, QObject*)),
            Qt::DirectConnection);

    m_sqCOHashMutex.lock();
    m_sqCOHash.insert(m_key, this);
    m_sqCOHashMutex.unlock();
}

void ControlObject::privateValueChanged(double dValue, QObject* pSetter) {
    // Only emit valueChanged() if we did not originate this change.
    if (pSetter != this) {
        emit(valueChanged(dValue));
    } else {
        emit(valueChangedFromEngine(dValue));
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

// static
void ControlObject::getControls(QList<ControlObject*>* pControlList) {
    m_sqCOHashMutex.lock();
    for (QHash<ConfigKey, ControlObject*>::const_iterator it = m_sqCOHash.begin();
         it != m_sqCOHash.end(); ++it) {
        pControlList->push_back(it.value());
    }
    m_sqCOHashMutex.unlock();
}

// static
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
        m_pControl->setMidiParameter(o, v);
    }
}

double ControlObject::getValueToMidi() const {
    return m_pControl ? m_pControl->getMidiParameter() : 0.0;
}

void ControlObject::setValueFromThread(double dValue, QObject* pSender) {
    if (m_pControl) {
        m_pControl->set(dValue, pSender);
    }
}

double ControlObject::get() const {
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
