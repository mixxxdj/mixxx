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
#include "configobject.h"
#include "midiobject.h"

//Forward declaration(s)
class ControlObjectThread;

class MidiScriptEngine : public QThread {

    Q_OBJECT

public:
    MidiScriptEngine(MidiObject* midi_object);
    ~MidiScriptEngine();

    bool isReady();
    bool hasErrors(QString filename);
    const QStringList getErrors(QString filename);

    // Evaluate a script file
    bool evaluate(QString filepath);
    // Execute a particular function
    bool execute(QString function); 
    // Execute a particular function with all the data
    bool execute(QString function, char channel,
                 QString device, char control,
                 char value, MidiCategory category); 

    // Lookup registered script functions
    QStringList getScriptFunctions();

    /* DO NOT CALL DIRECTLY,
       SHOULD ONLY BE CALLED FROM WITHIN SCRIPTS */
    Q_INVOKABLE double getValue(QString group, QString name);
    Q_INVOKABLE void setValue(QString group, QString name, double newValue);
    Q_INVOKABLE bool connectControl(QString group, QString name,
                                    QString function, bool disconnect = false);
    Q_INVOKABLE void trigger(QString group, QString name);

public slots:
    void slotValueChanged(double value);
    
signals:
    void initialized();
    void sigEvaluate(QString filename);
    void sigExecute(QString function);
    void sigExecute(QString function, char channel,
                    QString device, char control,
                    char value, MidiCategory category);
                                                      
private slots:
    bool safeEvaluate(QString filename);
    bool safeExecute(QString function);
    bool safeExecute(QString function, char channel,
                     QString device, char control,
                     char value, MidiCategory category);

protected:
    void run();

private:
    void generateScriptFunctions(QString code);
    bool checkException();

    MidiObject *m_pMidiObject;
    QHash<ConfigKey, QString> m_connectedControls;
    QScriptEngine *m_pEngine;
    QStringList m_scriptFunctions;
    QMap<QString,QStringList> m_scriptErrors;
    
//     QHash<ConfigKey, ControlObjectThread*> m_controlCache;
};

#endif

