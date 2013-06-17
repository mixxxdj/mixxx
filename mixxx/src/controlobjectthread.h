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

#include "configobject.h"

class ControlDoublePrivate;
class ControlObject;

class ControlObjectThread : public QObject {
    Q_OBJECT
  public:
    ControlObjectThread(ControlObject *pControlObject, QObject* pParent=NULL);
    ControlObjectThread(ConfigKey key, QObject* pParent=NULL);
    virtual ~ControlObjectThread();

    void initialize(ConfigKey key);

    /** Called from update(); */
    void emitValueChanged();

    inline ConfigKey getKey() const {
        return m_key;
    }

    // Returns the value of the object. Thread safe, non-blocking.
    virtual double get();

    bool valid() const;

  public slots:
    // Set the control to a new value. Non-blocking.
    virtual void slotSet(double v);
    // Sets the control value to v. Thread safe, non-blocking.
    virtual void set(double v);
    // Resets the control to its default value. Thread safe, non-blocking.
    virtual void reset();

  signals:
    void valueChanged(double);
    // This means that the control value has changed as a result of a mutation
    // (set/add/sub/reset) originating from this object.
    void valueChangedByThis(double);

  protected slots:
    // Receives the value from the master control and re-emits either
    // valueChanged(double) or valueChangedByThis(double) based on pSetter.
    virtual void slotValueChanged(double v, QObject* pSetter);

  protected:
    ConfigKey m_key;
    // Pointer to connected control.
    ControlDoublePrivate* m_pControl;
};

#endif
