/***************************************************************************
                         controlobjectthread.cpp  -  description
                            -------------------
   begin                : Thu Sep 23 2004
   copyright            : (C) 2004 by Tue Haste Andersen
   email                : haste@diku.dk
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QApplication>
#include <QtDebug>

#include "controlobjectthread.h"
#include "control/control.h"

ControlObjectThread::ControlObjectThread(const QString& g, const QString& i, QObject* pParent)
        : QObject(pParent) {
    initialize(ConfigKey(g, i));
}

ControlObjectThread::ControlObjectThread(const char* g, const char* i, QObject* pParent)
        : QObject(pParent) {
    initialize(ConfigKey(g, i));
}

ControlObjectThread::ControlObjectThread(const ConfigKey& key, QObject* pParent)
        : QObject(pParent) {
    initialize(key);
}

void ControlObjectThread::initialize(const ConfigKey& key) {
    m_key = key;
    m_pControl = ControlDoublePrivate::getControl(key);
    if (m_pControl) {
        connect(m_pControl.data(), SIGNAL(valueChanged(double, QObject*)),
                this, SLOT(slotValueChanged(double, QObject*)),
                Qt::DirectConnection);
    }
}

ControlObjectThread::~ControlObjectThread() {
}

bool ControlObjectThread::connectValueChanged(const QObject* receiver,
        const char* method, Qt::ConnectionType type) {
    return connect((QObject*)this, SIGNAL(valueChanged(double)), receiver, method, type);
}

bool ControlObjectThread::connectValueChanged(
        const char* method, Qt::ConnectionType type) {
    return connect((QObject*)this, SIGNAL(valueChanged(double)), parent(), method, type);
}

QString ControlObjectThread::name() const {
    return m_pControl ? m_pControl->name() : QString();
}

QString ControlObjectThread::description() const {
    return m_pControl ? m_pControl->description() : QString();
}

double ControlObjectThread::get() {
    return m_pControl ? m_pControl->get() : 0.0;
}

double ControlObjectThread::getParameter() const {
    return m_pControl ? m_pControl->getParameter() : 0.0;
}

double ControlObjectThread::getParameterForValue(double value) const {
    return m_pControl ? m_pControl->getParameterForValue(value) : 0.0;
}

void ControlObjectThread::slotSet(double v) {
    set(v);
}

void ControlObjectThread::set(double v) {
    if (m_pControl) {
        m_pControl->set(v, this);
    }
}

void ControlObjectThread::setParameter(double p) {
    if (m_pControl) {
        m_pControl->setParameter(p, this);
    }
}

void ControlObjectThread::reset() {
    if (m_pControl) {
        // NOTE(rryan): This is important. The originator of this action does
        // not know the resulting value so it makes sense that we should emit a
        // general valueChanged() signal even though the change originated from
        // us. For this reason, we provide NULL here so that the change is
        // broadcast as valueChanged() and not valueChangedByThis().
        m_pControl->reset();
    }
}

void ControlObjectThread::emitValueChanged() {
    emit(valueChanged(get()));
}

void ControlObjectThread::slotValueChanged(double v, QObject* pSetter) {
    if (pSetter != this) {
        // This is base implementation of this function without scaling
        emit(valueChanged(v));
    } else {
        emit(valueChangedByThis(v));
    }
}
