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
#include "controlobjectthread.h"
#include "controllers/midi/midimessage.h"
#include "controlobjectbase.h"

class QWidget;
class ConfigKey;

class ControlObject 
    : public QObject,
      private ControlObjectBase<double> {
    Q_OBJECT
  public:
    ControlObject();
    ControlObject(ConfigKey key, bool bIgnoreNops=true, bool track=false);
    ControlObject(const QString& group, const QString& item, bool bIgnoreNops=true);
    virtual ~ControlObject();
    /** Returns a pointer to the ControlObject matching the given ConfigKey */
    static ControlObject* getControl(const ConfigKey& key);
    static inline ControlObject* getControl(const QString& group, const QString& item) {
        ConfigKey key(group, item);
        return getControl(key);
    }

    // Adds all ControlObjects that currently exist to pControlList
    static void getControls(QList<ControlObject*>* pControlsList);

    /** Used to add a pointer to the corresponding ControlObjectThread of this ControlObject */
    void addProxy(ControlObjectThread *pControlObjectThread);
    // To get rid of a proxy when the corresponding object is being deleted for example
    void removeProxy(ControlObjectThread *pControlObjectThread);
    /** Return the key of the object */
    inline ConfigKey getKey() { return m_key; }
    // Returns the value of the ControlObject
    double get();
    // Sets the ControlObject value in a threadsave way and updates associated proxy objects. 
    void set(const double& value, bool emmitValueChanged = true);
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
    virtual void setDefaultValue(double dValue) {
        m_dDefaultValue = dValue;
    }
    virtual double defaultValue() const {
        return m_dDefaultValue;
    }

  signals:
    void valueChanged(double);

  public:
    // Called when a widget has changed value. Not thread safe.
    virtual void setValueFromMidi(MidiOpCode o, double v);
    // Called when another thread has changed value. Not thread safe.
    virtual void setValueFromThread(double dValue);

  protected:
    //TODO() ?? 
    double m_dDefaultValue;
    // Key of the object
    ConfigKey m_key;

  private:
    // Whether to ignore set/add/sub()'s which would have no effect
    bool m_bIgnoreNops;
    // Whether to track value changes with the stats framework.
    bool m_bTrack;
    QString m_trackKey;
    int m_trackType;
    int m_trackFlags;
    // List of associated proxy objects
    QList<ControlObjectThread*> m_qProxyList;
    // Mutex for the proxy list
    QMutex m_qProxyListMutex;

    // Hash of ControlObject instantiations
    static QHash<ConfigKey,ControlObject*> m_sqCOHash;
    // Mutex guarding access to the ControlObject hash
    static QMutex m_sqCOHashMutex;
};


#endif
