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
#include <qwidget.h>
#include <qevent.h>
#include <qptrlist.h>
#include <qaccel.h>
#include <qptrqueue.h>
#include <qmutex.h>
#include "configobject.h"
#include "midiobject.h"

class ControlEngine;

struct ControlQueueEngineItem
{
    ControlEngine *ptr;
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
    /** Connect two control objects dest and src, so each time src is updated, so is dest */
    static bool connectControls(ConfigKey src, ConfigKey dest);
    /** Returns a pointer to the ControlObject matching the given ConfigKey */
    static ControlObject *getControl(ConfigKey key);
    /** Sets the midi config object */
    static void setMidiConfig(ConfigObject<ConfigValueMidi> *pMidiConfig);
    /** Sets the keyboard config object */
    static void setKbdConfig(ConfigObject<ConfigValueKbd> *pKbdConfig);
    /** Associates a QWidget with the ControlObject identified by a given ConfigKey */
    static void setWidget(QWidget *widget, ConfigKey key, bool emitOnDownPress=true, Qt::ButtonState state=Qt::NoButton);
    /** Associates a QWidget with the ControlObject. */
    void setWidget(QWidget *widget, bool emitOnDownPress=true, Qt::ButtonState state=Qt::NoButton);
    /** Used to set a pointer to the corresponding ControlEngine of this ControlObject */
    void setControlEngine(ControlEngine *pControlEngine);
    /** Return the value of the ControlObject */
    double getValue();
    /** Used to set a keyboard accelarator (up) for the ControlObject. Up and down directions are provided,
      * even though some ControlObjects does not distinguish between the two (ControlPushButton for
      * instance */
    virtual void setAccelUp(const QKeySequence key) {};
    /** Used to set a keyboard accelarator (down) for the ControlObject */
    virtual void setAccelDown(const QKeySequence key) {};
    /** Sets up parent widget. Used when setting up keyboard accelerators */
    static void setParentWidget(QWidget *pParentWidget);
    /** Syncronizes queue, by writing the values to ControlEngine objects. Non blocking.
      * Should be called from player thread. */
    static void syncControlEngineObjects();
    /** Called from main widget, when a key is pressed or released */
    void kbdPress(QKeySequence k, bool release);

signals:
    /** Signal sent when the widget has to be updated with a given value */
    void signalUpdateWidget(double);
    /** Signal sent when the ControlObject value has changed by others that the main application
      * thread */
    void signalUpdateApp(double);

protected:
    /** Method called internally when the value has been updated by Midi */
    void updateFromMidi();
    /** Method called internally when the value has been updated by the keyboard */
    void updateFromKeyboard();
    /** Method called internally when the value has been updated by a ControlEngine */
    void updateFromEngine();
    /** Method called internally when the value has been updated by a graphical widget */
    void updateFromWidget();
    /** Method called internally when the value has been updated by the main application thread */
    void updateFromApp();
    /** Method called when the associated ControlEngine object needs to be updated */
    virtual void updateEngine();
    /** Method called when the associated widget needs to be updated */
    virtual void updateWidget();
    /** Method called when the application thread needs to be updated */
    virtual void updateApp();
    /** Called when a MIDI event associated with this object is received */
    virtual void setValueFromMidi(MidiCategory c, int v);

public slots:
    /** Called when a event from an associated ControlEngine object is received */
    virtual void setValueFromEngine(double dValue);
    /** Called when a signal from the associated widget is received */
    virtual void setValueFromWidget(double dValue);
    /** Called when a keyboard event associated with this object is received */
    virtual void setValueFromKeyboard();
    /** Called when the value is changed by the main application thread */
    virtual void setValueFromApp(double dValue);

protected:
    /** Return pointer to parent widget */
    QWidget *getParentWidget();
    /** The actual value of the controller */
    double m_dValue;
    /** Pointer to MIDI config */
    static ConfigObject<ConfigValueMidi> *m_pMidiConfig;
    /** Pointer to keyboard config */
    static ConfigObject<ConfigValueKbd> *m_pKbdConfig;
    /** Queue used in syncronizing the value with the Player thread */
    static QPtrQueue<ControlQueueEngineItem> queue;
    /** Pointer to midi config option */
    ConfigOption<ConfigValueMidi> *m_pMidiConfigOption;
    /** Pointer to keyboard config option */
    ConfigOption<ConfigValueKbd> *m_pKbdConfigOption;
    /** Pointer to associated ControlEngine object */
    ControlEngine *m_pControlEngine;
    /** Keyboard accelerator */
    QAccel *m_pAccel;


private:
    /** Called when a ControlEventMidi event is received */
    static void midi(MidiCategory category, char channel, char control, char value);
    /** Event filter. Used to receive ControlEventMidi and ControlEventEngine events */
    bool eventFilter(QObject *, QEvent *);
    /** Pointer to parent widget, used when setting up keyboard accelerators */
    static QWidget *spParentWidget;
    /** List of ControlObject instantiations */
    static QPtrList<ControlObject> list;
    /** Mutex protecting access to the queue */
    static QMutex queueMutex;

};


#endif

