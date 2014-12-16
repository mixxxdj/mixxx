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
#include "control/control.h"
#include "util/stat.h"
#include "util/timer.h"

ControlObject::ControlObject() {
}

ControlObject::ControlObject(ConfigKey key, bool bIgnoreNops, bool bTrack,
                             bool bPersist) {
    initialize(key, bIgnoreNops, bTrack, bPersist);
}

ControlObject::~ControlObject() {
    if (m_pControl) {
        m_pControl->removeCreatorCO();
    }
}

void ControlObject::initialize(ConfigKey key, bool bIgnoreNops, bool bTrack,
                               bool bPersist) {
    m_key = key;

    // Don't bother looking up the control if key is NULL. Prevents log spew.
    if (!m_key.isNull()) {
        m_pControl = ControlDoublePrivate::getControl(m_key, true, this,
                                                      bIgnoreNops, bTrack,
                                                      bPersist);
    }

    // getControl can fail and return a NULL control even with the create flag.
    if (m_pControl) {
        connect(m_pControl.data(), SIGNAL(valueChanged(double, QObject*)),
                this, SLOT(privateValueChanged(double, QObject*)),
                Qt::DirectConnection);
    }
}

// slot
void ControlObject::privateValueChanged(double dValue, QObject* pSender) {
    // Only emit valueChanged() if we did not originate this change.
    if (pSender != this) {
        emit(valueChanged(dValue));
    } else {
        emit(valueChangedFromEngine(dValue));
    }
}

// static
ControlObject* ControlObject::getControl(const ConfigKey& key, bool warn) {
    //qDebug() << "ControlObject::getControl for (" << key.group << "," << key.item << ")";
    QSharedPointer<ControlDoublePrivate> pCDP = ControlDoublePrivate::getControl(key, warn);
    if (pCDP) {
        return pCDP->getCreatorCO();
    }
    return NULL;
}

void ControlObject::setValueFromMidi(MidiOpCode o, double v) {
    if (m_pControl) {
        m_pControl->setMidiParameter(o, v);
    }
}

double ControlObject::getMidiParameter() const {
    return m_pControl ? m_pControl->getMidiParameter() : 0.0;
}

// static
double ControlObject::get(const ConfigKey& key) {
    QSharedPointer<ControlDoublePrivate> pCop = ControlDoublePrivate::getControl(key);
    return pCop ? pCop->get() : 0.0;
}

double ControlObject::getParameter() const {
    return m_pControl ? m_pControl->getParameter() : 0.0;
}

double ControlObject::getParameterForValue(double value) const {
    return m_pControl ? m_pControl->getParameterForValue(value) : 0.0;
}

double ControlObject::getParameterForMidiValue(double midiValue) const {
    return m_pControl ? m_pControl->getParameterForMidiValue(midiValue) : 0.0;
}

void ControlObject::setParameter(double v) {
    if (m_pControl) {
        m_pControl->setParameter(v, this);
    }
}

void ControlObject::setParameterFrom(double v, QObject* pSender) {
    if (m_pControl) {
        m_pControl->setParameter(v, pSender);
    }
}

// static
void ControlObject::set(const ConfigKey& key, const double& value) {
    QSharedPointer<ControlDoublePrivate> pCop = ControlDoublePrivate::getControl(key);
    if (pCop) {
        pCop->set(value, NULL);
    }
}

bool ControlObject::connectValueChangeRequest(const QObject* receiver,
                                              const char* method,
                                              Qt::ConnectionType type) {
    bool ret = false;
    if (m_pControl) {
        ret = m_pControl->connectValueChangeRequest(receiver, method, type);
    }
    return ret;
}
