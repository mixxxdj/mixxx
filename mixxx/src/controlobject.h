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

#include "configobject.h"
#include "controllers/midi/midimessage.h"
#include "controlobjectbase.h"
#include "control.h"

class ControlObject : public QObject {
    Q_OBJECT
  public:
    ControlObject();
    ControlObject(ConfigKey key,
                  bool bIgnoreNops=true, bool bTrack=false);
    ControlObject(const QString& group, const QString& item,
                  bool bIgnoreNops=true, bool bTrack=false);
    virtual ~ControlObject();

    /** Returns a pointer to the ControlObject matching the given ConfigKey */
    static ControlObject* getControl(const ConfigKey& key);
    static inline ControlObject* getControl(const QString& group, const QString& item) {
        ConfigKey key(group, item);
        return getControl(key);
    }

    // Adds all ControlObjects that currently exist to pControlList
    static void getControls(QList<ControlObject*>* pControlsList);

    // Return the key of the object
    inline ConfigKey getKey() { return m_key; }
    // Returns the value of the ControlObject
    double get();
    // Sets the ControlObject value
    void set(const double& value);
    // Sets the default value
    void reset();
    // Add to value
    void add(double dValue);
    // Subtract from value
    void sub(double dValue);
    // Return a ControlObject value, corresponding to the widget input value.
    virtual double getValueFromWidget(double dValue);
    // Return a widget value corresponding to the ControlObject input value.
    virtual double getValueToWidget(double dValue);
    // get value (range 0..127)
    virtual double GetMidiValue();
    inline void setDefaultValue(double dValue) {
        if (m_pControl) {
            m_pControl->setDefaultValue(dValue);
        }
    }
    inline double defaultValue() const {
        return m_pControl ? m_pControl->defaultValue() : 0.0;
    }

  signals:
    void valueChanged(double);

  public:
    // Called when a widget has changed value.
    virtual void setValueFromMidi(MidiOpCode o, double v);
    // Called when another thread has changed value.
    virtual void setValueFromThread(double dValue, QObject* pSetter);

  protected:
    // Key of the object
    ConfigKey m_key;

  private slots:
    void privateValueChanged(double value, QObject* pSetter);

  private:
    inline bool ignoreNops() const {
        return m_pControl ? m_pControl->ignoreNops() : true;
    }

    ControlNumericPrivate* m_pControl;

    // Hash of ControlObject instantiations
    static QHash<ConfigKey,ControlObject*> m_sqCOHash;
    // Mutex guarding access to the ControlObject hash
    static QMutex m_sqCOHashMutex;
};


#endif
