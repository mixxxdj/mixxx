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


QWaitCondition ControlObjectThread::m_sqWait;
QMutex ControlObjectThread::m_sqMutex;
QQueue<ControlObjectThread*> ControlObjectThread::m_sqQueue;

ControlObjectThread::ControlObjectThread(ControlObject* pControlObject, QObject* pParent)
        : QObject(pParent),
          m_dValue(0.0)
        , m_pControlObject(pControlObject) {
    // Update associated ControlObject
    if (m_pControlObject == NULL) return;
    Q_ASSERT(m_pControlObject);
    m_pControlObject->addProxy(this);

    connect(m_pControlObject, SIGNAL(destroyed()), this, SLOT(slotParentDead()));

    // Initialize value
    m_dValue = m_pControlObject->get();
    emitValueChanged();
}

ControlObjectThread::~ControlObjectThread()
{
    if (m_pControlObject)
    {
        // Our parent is still around, make sure it doesn't send us any more events
        m_pControlObject->removeProxy(this);
    }
}

double ControlObjectThread::get()
{
    m_dataMutex.lock();
    double v = m_dValue;
    m_dataMutex.unlock();

    return v;
}

void ControlObjectThread::slotSet(double v)
{
    m_dataMutex.lock();
    m_dValue = v;
    m_dataMutex.unlock();

    updateControlObject();
}

bool ControlObjectThread::setExtern(double v)
{
    bool result = false;

    if (m_dataMutex.tryLock()) {
        m_dValue = v;
        result = true;
        m_dataMutex.unlock();
    }

    // if ( m_sqMutex.tryLock() )
    // {
    //     if( m_dataMutex.tryLock() )
    //     {
    //         m_sqQueue.enqueue(this);
    //         m_dValue = v;

    //         m_dataMutex.unlock();
    //         m_sqMutex.unlock();

    //         result = true;
    //     }
    //     else
    //     {
    //         m_sqMutex.unlock();
    //     }
    // }
    
    emitValueChanged();
    return result;
}

bool ControlObjectThread::update()
{
    bool result = false;

    m_sqMutex.lock();
    ControlObjectThread* p = NULL;
    if (!m_sqQueue.isEmpty())
        p = m_sqQueue.dequeue();
    m_sqMutex.unlock();

    if (p)
    {
        p->emitValueChanged();
        result = true;
    }

    return result;
}

void ControlObjectThread::emitValueChanged()
{
    emit(valueChanged(get()));
}

void ControlObjectThread::add(double v)
{
    m_dataMutex.lock();
    m_dValue += v;
    m_dataMutex.unlock();

    updateControlObject();
}

void ControlObjectThread::sub(double v)
{
    m_dataMutex.lock();
    m_dValue -= v;
    m_dataMutex.unlock();

    updateControlObject();
}

void ControlObjectThread::updateControlObject()
{
    m_pControlObject->queueFromThread(get(), this);
}

void ControlObjectThread::slotParentDead()
{
    // Now we've got a chance of avoiding segfaults with judicious
    // use of if(m_pControlObject)
    m_pControlObject = 0;
}

ControlObject* ControlObjectThread::getControlObject()
{
   return m_pControlObject;
}
