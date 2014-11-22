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

#include <QObject>

#include "configobject.h"
#include "control/control.h"

class ControlObjectThread : public QObject {
    Q_OBJECT
  public:
    ControlObjectThread(const QString& g, const QString& i, QObject* pParent=NULL);
    ControlObjectThread(const char* g, const char* i, QObject* pParent=NULL);
    ControlObjectThread(const ConfigKey& key, QObject* pParent=NULL);
    virtual ~ControlObjectThread();

    void initialize(const ConfigKey& key);

    bool connectValueChanged(const QObject* receiver,
            const char* method, Qt::ConnectionType type = Qt::AutoConnection);
    bool connectValueChanged(
            const char* method, Qt::ConnectionType type = Qt::AutoConnection);

    QString name() const;
    QString description() const;

    /** Called from update(); */
    inline void emitValueChanged() {
        emit(valueChanged(get()));
    }

    inline ConfigKey getKey() const { return m_key; }
    inline bool valid() const { return m_pControl != NULL; }

    // Returns the value of the object. Thread safe, non-blocking.
    inline double get() {
        return m_pControl ? m_pControl->get() : 0.0;
    }

    // Returns the normalized parameter of the object. Thread safe, non-blocking.
    inline double getParameter() const {
        return m_pControl ? m_pControl->getParameter() : 0.0;
    }

    // Set the normalized parameter of the object. Thread safe, non-blocking.
    inline void setParameter(double p) {
        if (m_pControl) {
            m_pControl->setParameter(p, this);
        }
    }

    inline double getParameterForValue(double value) const {
        return m_pControl ? m_pControl->getParameterForValue(value) : 0.0;
    }

    // Returns the normalized parameter of the object. Thread safe, non-blocking.
    inline double getDefault() const {
        return m_pControl ? m_pControl->defaultValue() : 0.0;
    }

  public slots:
    // Set the control to a new value. Non-blocking.
    inline void slotSet(double v) {
        set(v);
    }

    // Sets the control value to v. Thread safe, non-blocking.
    inline void set(double v) {
        if (m_pControl) {
            m_pControl->set(v, this);
        }
    }

    // Resets the control to its default value. Thread safe, non-blocking.
    inline void reset() {
        if (m_pControl) {
            // NOTE(rryan): This is important. The originator of this action does
            // not know the resulting value so it makes sense that we should emit a
            // general valueChanged() signal even though the change originated from
            // us. For this reason, we provide NULL here so that the change is
            // broadcast as valueChanged() and not valueChangedByThis().
            m_pControl->reset();
        }
    }

  signals:
    void valueChanged(double);
    // This means that the control value has changed as a result of a mutation
    // (set/add/sub/reset) originating from this object.
    void valueChangedByThis(double);

  protected slots:
    // Receives the value from the master control and re-emits either
    // valueChanged(double) or valueChangedByThis(double) based on pSetter.
    inline void slotValueChanged(double v, QObject* pSetter) {
        if (pSetter != this) {
            // This is base implementation of this function without scaling
            emit(valueChanged(v));
        } else {
            emit(valueChangedByThis(v));
        }
    }

  protected:
    ConfigKey m_key;
    // Pointer to connected control.
    QSharedPointer<ControlDoublePrivate> m_pControl;
};

#endif
