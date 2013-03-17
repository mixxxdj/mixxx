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


ControlObject::ControlObject()
        : ControlObjectBase<double>(),
          m_dDefaultValue(0),
          m_bIgnoreNops(true) {
    set(m_dDefaultValue);
}

ControlObject::ControlObject(ConfigKey key, bool bIgnoreNops, bool track)
        : m_dDefaultValue(0),
          m_key(key),
          m_bIgnoreNops(bIgnoreNops),
          m_bTrack(track),
          m_trackKey("control " + m_key.group + "," + m_key.item),
          m_trackType(Stat::UNSPECIFIED),
          m_trackFlags(Stat::COUNT | Stat::SUM | Stat::AVERAGE |
                       Stat::SAMPLE_VARIANCE | Stat::MIN | Stat::MAX) {
    set(m_dDefaultValue),
    m_sqCOHashMutex.lock();
    m_sqCOHash.insert(m_key, this);
    m_sqCOHashMutex.unlock();

    if (m_bTrack) {
        // TODO(rryan): Make configurable.
        Stat::track(m_trackKey, static_cast<Stat::StatType>(m_trackType),
                    static_cast<Stat::ComputeFlags>(m_trackFlags), m_dDefaultValue);
    }
}

ControlObject::ControlObject(const QString& group, const QString& item, bool bIgnoreNops)
        : m_dDefaultValue(0),
          m_key(group, item),
          m_bIgnoreNops(bIgnoreNops) {
    set(m_dDefaultValue);
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

void ControlObject::setValueFromMidi(MidiOpCode o, double v)
{
    Q_UNUSED(o);
    set(v);
}

double ControlObject::GetMidiValue()
{
    return get();
}

void ControlObject::setValueFromThread(double dValue)
{
    set(dValue);
}

void ControlObject::add(double dValue)
{
    if (m_bIgnoreNops && !dValue) {
        return;
    }
    set(get() + dValue);
}

void ControlObject::sub(double dValue)
{
    if (m_bIgnoreNops && !dValue) {
        return;
    }
    set(get() - dValue);
}

double ControlObject::getValueFromWidget(double v)
{
    return v;
}

double ControlObject::getValueToWidget(double v)
{
    return v;
}

double ControlObject::get() {
    return getValue();
}

void ControlObject::reset() {
    set(m_dDefaultValue);
}

void ControlObject::set(const double& value, bool emitValueChanged) {
    if (m_bIgnoreNops) {
        if (get() == value) {
            return;
        }
    }
    setValue(value);
    if (emitValueChanged) {
        emit(valueChanged(value));
        if (m_bTrack) {
            Stat::track(m_trackKey, static_cast<Stat::StatType>(m_trackType),
                        static_cast<Stat::ComputeFlags>(m_trackFlags), value);
        }
    }
}
