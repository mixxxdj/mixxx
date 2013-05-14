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

class ControlNumericPrivate;
class ControlObject;

class ControlObjectThread : public QObject {
    Q_OBJECT
  public:
    ControlObjectThread(ControlObject *pControlObject, QObject* pParent=NULL);
    virtual ~ControlObjectThread();

    /** Called from update(); */
    void emitValueChanged();

    inline ConfigKey getKey() const {
        return m_key;
    }

    // Returns the value of the object. Thread safe, non-blocking.
    virtual double get();

  public slots:
    // Set the control to a new value. Non-blocking.
    virtual void slotSet(double v);
    // Adds v to the control value. Thread safe, non-blocking.
    virtual void add(double v);
    // Subtracts v from the control value. Thread safe, non-blocking.
    virtual void sub(double v);
    // Sets the control value to v. Thread safe, non-blocking.
    virtual void set(double v);
    // Resets the control to its default value. Thread safe, non-blocking.
    virtual void reset();

  signals:
    void valueChanged(double);

  protected slots:
    // Receives the Value from the parent and may scales the vale and re-emit it again
    virtual void slotValueChanged(double v, QObject* pSetter);

  private slots:
    // Called when the associated ControlObject is dead.
    // TODO(rryan): Remove when we separate validators/translators from CO.
    void slotControlObjectDead();

  protected:
    ConfigKey m_key;
    // Pointer to connected control.
    ControlObject* m_pControlObject;
    ControlNumericPrivate* m_pControl;
};

#endif
