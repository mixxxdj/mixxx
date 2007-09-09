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

#include <qapplication.h>
#include "controlobjectthread.h"
#include "controlobject.h"
#include "engineobject.h"

QWaitCondition ControlObjectThread::m_sqWait;
QMutex ControlObjectThread::m_sqMutex;
Q3PtrQueue<ControlObjectThread> ControlObjectThread::m_sqQueue;

ControlObjectThread::ControlObjectThread(ControlObject * pControlObject)
{
    m_pControlObject = pControlObject;

    // Update associated ControlObject
    Q_ASSERT(m_pControlObject);
    m_pControlObject->addProxy(this);

    connect(m_pControlObject, SIGNAL(destroyed()), this, SLOT(slotParentDead()));

    // Initialize value
    m_dValue = m_pControlObject->get();
    emitValueChanged();
}

ControlObjectThread::~ControlObjectThread()
{
    if (m_pControlObject) {
        // Our parent is still around, make sure it doesn't send us any more events
        m_pControlObject->removeProxy(this);
    }
}

double ControlObjectThread::get()
{
    double v;
    m_sqMutex.lock();
    v = m_dValue;
    m_sqMutex.unlock();
    return v;
}

void ControlObjectThread::slotSet(double v)
{
    m_sqMutex.lock();
    m_dValue = v;
    m_sqMutex.unlock();
    updateControlObject();
}

bool ControlObjectThread::setExtern(double v)
{
    if (m_sqMutex.tryLock())
    {
        m_sqQueue.enqueue(this);
        m_dValue = v;
        m_sqMutex.unlock();
        return true;
    }
    return false;
}

bool ControlObjectThread::update()
{
    ControlObjectThread * p;
    m_sqMutex.lock();
    p = m_sqQueue.dequeue();
    m_sqMutex.unlock();

    if (p)
    {
        p->emitValueChanged();
        return true;
    }
    else
        return false;
}

void ControlObjectThread::emitValueChanged()
{
    emit(valueChanged(get()));
}

void ControlObjectThread::add(double v)
{
    m_sqMutex.lock();
    m_dValue += v;
    m_sqMutex.unlock();

    updateControlObject();
}

void ControlObjectThread::sub(double v)
{
    m_sqMutex.lock();
    m_dValue -= v;
    m_sqMutex.unlock();

    updateControlObject();
}

void ControlObjectThread::updateControlObject()
{
    m_pControlObject->queueFromThread(get(), this);
}

void ControlObjectThread::slotParentDead() {

    // Now we've got a chance of avoiding segfaults with judicious
    // use of if(m_pControlObject)
    m_pControlObject = 0;
}

