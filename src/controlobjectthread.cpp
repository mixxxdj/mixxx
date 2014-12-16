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
    // Don't bother looking up the control if key is NULL. Prevents log spew.
    if (!key.isNull()) {
        m_pControl = ControlDoublePrivate::getControl(key);
    }
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
