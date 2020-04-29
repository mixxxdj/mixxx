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

#include <QObject>
#include <QEvent>
#include <QMutex>

#include "preferences/usersettings.h"
#include "controllers/midi/midimessage.h"
#include "control/control.h"

class ControlObject : public QObject {
    Q_OBJECT
  public:
    ControlObject();

    // bIgnoreNops: Don't emit a signal if the CO is set to its current value.
    // bTrack: Record statistics about this control.
    // bPersist: Store value on exit, load on startup.
    // defaultValue: default value of CO. If CO is persistent and there is no valid
    //               value found in the config, this is also the initial value.
    ControlObject(ConfigKey key,
                  bool bIgnoreNops = true, bool bTrack = false,
                  bool bPersist = false, double defaultValue = 0.0);
    virtual ~ControlObject();

    // Returns a pointer to the ControlObject matching the given ConfigKey
    static ControlObject* getControl(const ConfigKey& key, bool warn = true);
    static inline ControlObject* getControl(const QString& group, const QString& item, bool warn = true) {
        ConfigKey key(group, item);
        return getControl(key, warn);
    }
    static inline ControlObject* getControl(const char* group, const char* item, bool warn = true) {
        ConfigKey key(group, item);
        return getControl(key, warn);
    }

    QString name() const {
        return m_pControl ?  m_pControl->name() : QString();
    }

    void setName(const QString& name) {
        if (m_pControl) {
            m_pControl->setName(name);
        }
    }

    const QString description() const {
        return m_pControl ?  m_pControl->description() : QString();
    }

    void setDescription(const QString& description) {
        if (m_pControl) {
            m_pControl->setDescription(description);
        }
    }

    // Return the key of the object
    inline ConfigKey getKey() const {
        return m_key;
    }

    // Returns the value of the ControlObject
    inline double get() const {
        return m_pControl ? m_pControl->get() : 0.0;
    }

    // Returns the bool interpretation of the ControlObject
    inline bool toBool() const {
        return get() > 0.0;
    }

    // Instantly returns the value of the ControlObject
    static double get(const ConfigKey& key);

    // Sets the ControlObject value. May require confirmation by owner.
    inline void set(double value) {
        if (m_pControl) {
            m_pControl->set(value, this);
        }
    }

    // Sets the ControlObject value and confirms it.
    inline void setAndConfirm(double value) {
        if (m_pControl) {
            m_pControl->setAndConfirm(value, this);
        }
    }

    // Forces the control to 'value', regardless of whether it has a change
    // request handler attached (identical to setAndConfirm).
    inline void forceSet(double value) {
        setAndConfirm(value);
    }

    // Instantly sets the value of the ControlObject
    static void set(const ConfigKey& key, const double& value);

    // Sets the default value
    inline void reset() {
        if (m_pControl) {
            m_pControl->reset();
        }
    }

    inline void setDefaultValue(double dValue) {
        if (m_pControl) {
            m_pControl->setDefaultValue(dValue);
        }
    }
    inline double defaultValue() const {
        return m_pControl ? m_pControl->defaultValue() : 0.0;
    }

    // Returns the parameterized value of the object. Thread safe, non-blocking.
    virtual double getParameter() const;

    // Returns the parameterized value of the object. Thread safe, non-blocking.
    virtual double getParameterForValue(double value) const;

    // Returns the parameterized value of the object. Thread safe, non-blocking.
    virtual double getParameterForMidi(double midiValue) const;

    // Sets the control parameterized value to v. Thread safe, non-blocking.
    virtual void setParameter(double v);

    // Sets the control parameterized value to v. Thread safe, non-blocking.
    virtual void setParameterFrom(double v, QObject* pSender = NULL);

    // Connects a Qt slot to a signal that is delivered when a new value change
    // request arrives for this control.
    // Qt::AutoConnection: Qt ensures that the signal slot is called from the
    // thread where the receiving object was created
    // You need to use Qt::DirectConnection for the engine objects, since the
    // audio thread has no Qt event queue. But be a ware of race conditions in this case.
    // ref: http://qt-project.org/doc/qt-4.8/qt.html#ConnectionType-enum
    template <typename Receiver, typename Slot>
    bool connectValueChangeRequest(Receiver receiver, Slot func,
                                   Qt::ConnectionType type = Qt::AutoConnection) {
        bool ret = false;
        if (m_pControl) {
          ret = m_pControl->connectValueChangeRequest(receiver, func, type);
        }
        return ret;
    }

    // Installs a value-change request handler that ignores all sets.
    void setReadOnly();

  signals:
    void valueChanged(double);

  public:
    // DEPRECATED: Called to set the control value from the controller
    // subsystem.
    virtual void setValueFromMidi(MidiOpCode o, double v);
    virtual double getMidiParameter() const;

  protected:
    // Key of the object
    ConfigKey m_key;
    QSharedPointer<ControlDoublePrivate> m_pControl;

  private slots:
    void privateValueChanged(double value, QObject* pSetter);
    void readOnlyHandler(double v);

  private:
    inline bool ignoreNops() const {
        return m_pControl ? m_pControl->ignoreNops() : true;
    }
};

#endif
