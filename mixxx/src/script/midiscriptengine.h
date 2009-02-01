/***************************************************************************
                          midiscriptengine.h  -  description
                          -------------------
    begin                : Fri Dec 12 2008
    copyright            : (C) 2008 by Sean M. Pappalardo
    email                : pegasus@renegadetech.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef MIDISCRIPTENGINE_H
#define MIDISCRIPTENGINE_H

#include <QEvent>
#include <QtScript>
#include <QMutex>
#include "../configobject.h"
#include "../midiobject.h"

const QEvent::Type EXECUTE_EVENT_TYPE = QEvent::Type(QEvent::User + 128);

class ExecuteEvent : public QEvent {
    public:
    ExecuteEvent(QString function);
    ExecuteEvent(QString function,
                 char channel,
                 QString device,
                 char control,
                 char value,
                 MidiCategory category);

    inline bool isSimple() {
        return m_bSimple;
    }
    
    inline QString function() {
        return m_function;
    }
    
    inline char channel() {
        return m_channel;
    }
    
    inline QString device() {
        return m_device;;
    }
    
    inline char control() {
        return m_control;
    }
    
    inline char value() {
        return m_value;
    }
    
    inline MidiCategory category() {
        return m_category;
    }

private:
    bool m_bSimple;
    QString m_function;
    char m_channel;
    QString m_device;
    char m_control;
    char m_value;
    MidiCategory m_category;
};


//Forward declaration(s)
class ControlObjectThread;
// class MidiObject;

class MidiScriptEngine : public QThread {

    Q_OBJECT

public:
    MidiScriptEngine();
    ~MidiScriptEngine();

    bool event(QEvent* e);

    QScriptValue engineGlobalObject;
    QScriptEngine *getEngine();

    void clearCode();
    bool loadScript(QString filepath);
    QString getResult();
    QString getLastFilepath();

    void evaluateScript();  // The whole thing
    bool isGood();  // The whole thing
    bool isReady();
    
    bool execute(QString function); // A particular function
    bool execute(QString function, char channel, QString device, char control, char value, MidiCategory category); // A particular function with all the data
    
    QStringList getFunctionList();
    
    bool checkException();  // The most recent operation

    Q_INVOKABLE double getValue(QString group, QString name);
    Q_INVOKABLE void setValue(QString group, QString name, double newValue);
    Q_INVOKABLE bool connectControl(QString group, QString name, QString function, bool disconnect = false);
    Q_INVOKABLE void trigger(QString group, QString name);

    public slots:
    void slotValueChanged(double value);
protected:
    QMutex m_mutex;
    void run();

private:
    bool safeExecute(QString function, char channel, QString device, char control, char value, MidiCategory category);
    bool safeExecute(QString function);
    QScriptEngine *m_pEngine;
    QScriptValue m_scriptFunction;
    QString m_lastFilepath;
    QString m_scriptCode;
    bool m_scriptGood;
    QHash<ConfigKey, ControlObjectThread*> m_controlCache;
    QHash<ConfigKey, QString> m_connectedControls;
};
#endif

