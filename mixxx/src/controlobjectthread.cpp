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
#include "controlobject.h"
#include "control/control.h"

ControlObjectThread::ControlObjectThread(ControlObject* pControlObject, QObject* pParent)
        : QObject(pParent) {
    initialize(pControlObject ? pControlObject->getKey() : ConfigKey());
}

ControlObjectThread::ControlObjectThread(ConfigKey key, QObject* pParent)
        : QObject(pParent) {
    initialize(key);
}

void ControlObjectThread::initialize(ConfigKey key) {
    m_key = key;
    m_pControl = ControlDoublePrivate::getControl(m_key, false);
    if (m_pControl) {
        connect(m_pControl, SIGNAL(valueChanged(double, QObject*)),
                this, SLOT(slotValueChanged(double, QObject*)),
                Qt::DirectConnection);
    }
}

ControlObjectThread::~ControlObjectThread() {
}

bool ControlObjectThread::valid() const {
    return m_pControl != NULL;
}

double ControlObjectThread::get() {
    return m_pControl ? m_pControl->get() : 0.0;
}

void ControlObjectThread::slotSet(double v) {
    set(v);
}

void ControlObjectThread::set(double v) {
    if (m_pControl) {
        m_pControl->set(v, this);
    }
}

void ControlObjectThread::reset() {
    if (m_pControl) {
        // NOTE(rryan): This is important. The originator of this action does
        // not know the resulting value so it makes sense that we should emit a
        // general valueChanged() signal even though the change originated from
        // us. For this reason, we provide NULL here so that the change is
        // broadcast as valueChanged() and not valueChangedByThis().
        m_pControl->reset(NULL);
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
