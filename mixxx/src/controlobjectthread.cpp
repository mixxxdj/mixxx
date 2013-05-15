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
#include "control.h"

ControlObjectThread::ControlObjectThread(ControlObject* pControlObject, QObject* pParent)
        : QObject(pParent),
          m_key(pControlObject ? pControlObject->getKey() : ConfigKey()),
          m_pControl(NULL) {
    if (pControlObject) {
        m_pControl = ControlNumericPrivate::getControl(pControlObject->getKey(), false);
    }
    if (m_pControl) {
        connect(m_pControl, SIGNAL(valueChanged(double, QObject*)),
                this, SLOT(slotValueChanged(double, QObject*)),
                Qt::DirectConnection);
    }
}

ControlObjectThread::~ControlObjectThread() {
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
        m_pControl->reset(NULL);
    }
}

void ControlObjectThread::add(double v) {
    if (m_pControl) {
        m_pControl->add(v, this);
    }
}

void ControlObjectThread::sub(double v) {
    if (m_pControl) {
        m_pControl->sub(v, this);
    }
}

void ControlObjectThread::emitValueChanged() {
    emit(valueChanged(get()));
}

void ControlObjectThread::slotValueChanged(double v, QObject* pSetter) {
    if (pSetter != this) {
        // This is base implementation of this function without scaling
        emit(valueChanged(v));
    }
}
