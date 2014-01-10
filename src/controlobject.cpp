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

ControlObject::ControlObject()
        : m_pControl(NULL) {
}

ControlObject::ControlObject(ConfigKey key, bool bIgnoreNops, bool bTrack)
        : m_pControl(NULL) {
    initialize(key, bIgnoreNops, bTrack);
}

ControlObject::~ControlObject() {
    m_pControl->removeCreatorCO();
}

void ControlObject::initialize(ConfigKey key, bool bIgnoreNops, bool bTrack) {
    m_key = key;
    m_pControl = ControlDoublePrivate::getControl(m_key, true, this, bIgnoreNops, bTrack);
    connect(m_pControl.data(), SIGNAL(valueChanged(double, QObject*)),
            this, SLOT(privateValueChanged(double, QObject*)),
            Qt::DirectConnection);
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

double ControlObject::get() const {
    return m_pControl ? m_pControl->get() : 0.0;
}

// static
double ControlObject::get(const ConfigKey& key) {
    QSharedPointer<ControlDoublePrivate> pCop = ControlDoublePrivate::getControl(key);
    return pCop ? pCop->get() : 0.0;
}

void ControlObject::reset() {
    if (m_pControl) {
        m_pControl->reset();
    }
}

void ControlObject::set(double value) {
    if (m_pControl) {
        m_pControl->set(value, this);
    }
}

void ControlObject::setAndConfirm(double value) {
    if (m_pControl) {
        m_pControl->setAndConfirm(value, this);
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
