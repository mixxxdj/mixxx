/***************************************************************************
                          controlobject.h  -  description
                             -------------------
    begin                : Wed Feb 20 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONTROLOBJECT_H
#define CONTROLOBJECT_H

#include <qobject.h>
#include <qevent.h>
#include <qptrlist.h>
//#include <qaccel.h>
#include <qptrqueue.h>
#include <qmutex.h>
#include "configobject.h"
#include "midiobject.h"
#include "controlobjectthread.h"

class QWidget;
class ConfigKey;

struct QueueObjectThread
{
    ControlObjectThread *pControlObjectThread;
    ControlObject *pControlObject;
    double value;
};

struct QueueObjectMidi
{
    ControlObject *pControlObject;
    MidiCategory category;
    double value;
};

/**
  * ControlObjects is used as a way to share controller values between controllers, GUI and
  * the sound engine. Whenever the value is changed by either a connected widget or a ControlEventMidi
  * the emitValueChanged method is called which syncronizes the new value with the player thread
  * using the semaphore protected queue.
  *
  * The player thread has a corresponding ControlEngine object for each ControlObject. The
  * ControlEngine object updates the ControlObject by queueing the values, and sending them as an
  * event to the ControlObject.
  *
  *@author Tue and Ken Haste Andersen
  */

class ControlObject : public QObject
{
    Q_OBJECT
public:
    ControlObject();
    ControlObject(ConfigKey key);
    ~ControlObject();
    /** Connect two control objects dest and src, so each time src is updated, so is dest. */
    static bool connectControls(ConfigKey src, ConfigKey dest);
    /** Disonnect a control object. */
    static bool disconnectControl(ConfigKey key);
    /** Returns a pointer to the ControlObject matching the given ConfigKey */
    static ControlObject *getControl(ConfigKey key);
    /** Used to add a pointer to the corresponding ControlObjectThread of this ControlObject */
    void addProxy(ControlObjectThread *pControlObjectThread);
    /** Update proxies, execep the one given a pointer to. Returns true if all updates
      * happend, otherwise false. */
    bool updateProxies(ControlObjectThread *pProxyNoUpdate=0);
    /** Return the key of the object */
    ConfigKey getKey();
    /** Return the value of the ControlObject */
    double get();
    /** Add to value. Not thread safe. */
    void add(double dValue);
    /** Subtract from value. Not thread safe. */
    void sub(double dValue);
    /** Syncronizes all ControlObjects with their corresponding proxies. */
    static void sync();
    /** Queue a control change from a widget. Thread safe. Blocking. */
    void queueFromThread(double dValue, ControlObjectThread *pControlObjectThread=0);
    /** Queue a control change from MIDI. Thread safe. Blocking. */
    void queueFromMidi(MidiCategory c, double v);
    /** Return a ControlObject value, corresponding to the widget input value. Thread safe. */
    virtual double getValueFromWidget(double dValue);
    /** Return a widget value corresponding to the ControlObject input value. Thread safe. */
    virtual double getValueToWidget(double dValue);
    /** get value (range 0..127) **/
    virtual double GetMidiValue();

public slots:
    /** Sets the value of the object and updates associated proxy objects. Not thread safe. */
    void set(double dValue);

signals:
    void valueChanged(double);
    void valueChangedFromEngine(double);

protected:
    /** Sets the value of the object. Not thread safe. */
    virtual void setValueFromEngine(double dValue);
    /** Called when a widget has changed value. Not thread safe. */
    virtual void setValueFromMidi(MidiCategory, double v);
    /** Called when another thread has changed value. Not thread safe. */
    virtual void setValueFromThread(double dValue);

protected:
    /** The actual value of the controller */
    double m_dValue;
    /** Key of the object */
    ConfigKey m_Key;

private:
    /** List of associated proxy objects */
    QPtrList<ControlObjectThread> m_qProxyList;
    /** List of ControlObject instantiations */
    static QPtrList<ControlObject> m_sqList;
    /** Mutex protecting access to the queues */
    static QMutex m_sqQueueMutexMidi, m_sqQueueMutexThread;
    /** Queue holding control changes from MIDI */
    static QPtrQueue<QueueObjectMidi> m_sqQueueMidi;
    /** Queues holding control changes from other application threads and from widgets */
    static QPtrQueue<QueueObjectThread> m_sqQueueThread;
    /** Queue holding ControlObjects that has changed, but not been syncronized with it's
     * associated ControlObjectProxy objects. */
    static QPtrQueue<ControlObject> m_sqQueueChanges;
};


#endif
