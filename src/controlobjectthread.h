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

class ControlDoublePrivate;

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
            const char* method, Qt::ConnectionType type = Qt::AutoConnection );

    QString name() const;
    QString description() const;

    /** Called from update(); */
    void emitValueChanged();

    inline ConfigKey getKey() const { return m_key; }
    inline bool valid() const { return m_pControl != NULL; }

    // Returns the value of the object. Thread safe, non-blocking.
    virtual double get();

    // Returns the normalized parameter of the object. Thread safe, non-blocking.
    virtual double getParameter() const;

    // Set the normalized parameter of the object. Thread safe, non-blocking.
    virtual void setParameter(double p);

    double getParameterForValue(double value) const;

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
    QSharedPointer<ControlDoublePrivate> m_pControl;
};

#endif
