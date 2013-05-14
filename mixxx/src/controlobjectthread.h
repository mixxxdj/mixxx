/***************************************************************************
                          controlobjectthread.h  -  description
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

#ifndef CONTROLOBJECTTHREAD_H
#define CONTROLOBJECTTHREAD_H

#include <qmutex.h>
#include <qobject.h>
#include <qmutex.h>
#include <qwaitcondition.h>
#include <QQueue>

class ControlObject;

class ControlObjectThread : public QObject {
    Q_OBJECT
  public:
    ControlObjectThread(ControlObject *pControlObject, QObject* pParent=NULL);
    virtual ~ControlObjectThread();

    // Returns the value of the object. Thread safe, non-blocking.
    virtual double get();
    // Adds v to the control value. Thread safe, non-blocking.
    virtual void add(double v);
    // Subtracts v from the control value. Thread safe, non-blocking.
    virtual void sub(double v);
    /** Called from update(); */
    void emitValueChanged();
    // FIXME: Dangerous GED hack
    ControlObject* getControlObject();

  public slots:
    // Set the control to a new value. Non-blocking.
    virtual void slotSet(double v);

    // The danger signal! This is for safety in wierd shutdown scenarios where the
    // ControlObject dies to avoid segfaults.
    void slotParentDead();

  signals:
    void valueChanged(double);

  private slots:
    // Receives the Value from the parent and may scales the vale and re-emit it again
    virtual void slotValueChanged(double v, QObject* pSetter);

  protected:
    // Pointer to connected controls.
    ControlObject *m_pControlObject;
};

#endif
