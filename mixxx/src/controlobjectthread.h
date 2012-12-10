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

    /** Returns the value of the object. Thread safe, blocking */
    virtual double get();
    /** Setting the value from an external controller. This happen when a ControlObject has
      * changed and its value is syncronized with this object. Thread safe, non blocking. Returns
      * true if successful, otherwise false. Thread safe, non blocking. */
    virtual bool setExtern(double v);
    /** Adds a value to the value property of the ControlEngine. Notification in a similar way
     * to set. Thread safe, blocking. */
    virtual void add(double v);
    /** Subtracts a value to the value property. Notification in a similar way
     * to set. Thread safe, blocking. */
    virtual void sub(double v);
    /** Called from update(); */
    void emitValueChanged();


    // FIXME: Dangerous GED hack
    ControlObject* getControlObject();

public slots:
    /** The value is changed by the engine, and the corresponding ControlObject is updated.
      * Thread safe, blocking. */
    virtual void slotSet(double v);

    // The danger signal! This is for safety in wierd shutdown scenarios where the
    // ControlObject dies to avoid segfaults.
    void slotParentDead();

signals:
    void valueChanged(double);

protected:
    /** The actual value of the object */
    double m_dValue;
    /** Mutex controlling access to non-static members*/
    QMutex m_dataMutex;
    /** Pointer to corresponding ControlObject */
    ControlObject *m_pControlObject;
    /** Update corresponding ControlObject to the value */
    virtual void updateControlObject(double v);
};

#endif
