/***************************************************************************
                          midiscriptengine.h  -  description
                          -------------------
    begin                : Fri Dec 12 2008
    copyright            : (C) 2008-2010 by Sean M. Pappalardo
    email                : spappalardo@mixxx.org
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
#include <QMessageBox>
#include "configobject.h"
#include "midimessage.h"
#include "pitchfilter.h"
class MidiDevice;

//Forward declaration(s)
class ControlObjectThread;

class MidiScriptEngine : public QThread {

    Q_OBJECT

public:
    MidiScriptEngine(MidiDevice* midiDevice);
    ~MidiScriptEngine();

    bool isReady();
    bool hasErrors(QString filename);
    const QStringList getErrors(QString filename);

    // Lookup registered script functions
    QStringList getScriptFunctions();

    /* DO NOT CALL DIRECTLY,
       SHOULD ONLY BE CALLED FROM WITHIN SCRIPTS */
    Q_INVOKABLE double getValue(QString group, QString name);
    Q_INVOKABLE void setValue(QString group, QString name, double newValue);
    Q_INVOKABLE bool connectControl(QString group, QString name,
                                    QString function, bool disconnect = false);
    Q_INVOKABLE void trigger(QString group, QString name);
    Q_INVOKABLE void log(QString message);
    Q_INVOKABLE int beginTimer(int interval, QString scriptCode, bool oneShot = false);
    Q_INVOKABLE void stopTimer(int timerId);
    Q_INVOKABLE void scratchEnable(int deck, int intervalsPerRev, float rpm, float alpha, float beta);
    Q_INVOKABLE void scratchTick(int deck, int interval);
    Q_INVOKABLE void scratchDisable(int deck);

public slots:
    void slotValueChanged(double value);
    // Evaluate a script file
    bool evaluate(QString filepath);
    // Execute a particular function
    bool execute(QString function); 
    // Execute a particular function with a data string (e.g. a device ID)
    bool execute(QString function, QString data);
    // Execute a particular function with a data buffer (e.g. a SysEx message)
    bool execute(QString function, const unsigned char data[], unsigned int length);
    // Execute a particular function with all the data
    bool execute(QString function, char channel,
                 char control, char value, MidiStatusByte status, QString group);
    void loadScriptFiles(QList<QString> scriptFileNames);
    void initializeScripts(QList<QString> scriptFunctionPrefixes);
    void gracefulShutdown(QList<QString> scriptFunctionPrefixes);
    
signals:
    void initialized();
    void resetController();

protected:
    void run();
    void timerEvent(QTimerEvent *event);

private slots:
    void errorDialogButton(QString key, QMessageBox::StandardButton button);
    
private:
    // Only call these with the scriptEngineLock
    bool safeEvaluate(QString scriptName, QList<QString> scriptPaths);
    bool internalExecute(QString scriptCode);
    bool safeExecute(QString function);
    bool safeExecute(QString function, QString data);
    bool safeExecute(QString function, const unsigned char data[], unsigned int length);
    bool safeExecute(QString function, char channel, 
                     char control, char value, MidiStatusByte status, QString group);
    void initializeScriptEngine();
    
    void scriptErrorDialog(QString detailedError);
    void generateScriptFunctions(QString code);
    bool checkException();
    void stopAllTimers();

    ControlObjectThread* getControlObjectThread(QString group, QString name);
        
    MidiDevice* m_pMidiDevice;
    bool m_midiDebug;
    QMultiHash<ConfigKey, QString> m_connectedControls;
    QScriptEngine *m_pEngine;
    QStringList m_scriptFunctions;
    QMap<QString,QStringList> m_scriptErrors;
    QMutex m_scriptEngineLock;
    QHash<ConfigKey, ControlObjectThread*> m_controlCache;
    QHash<int, QPair<QString, bool> > m_timers;
    
    // Scratching functions & variables
    void scratchProcess(int timerId);
    
    // 256 (default) available virtual decks is enough I would think.
    //  If more are needed at run-time, these will move to the heap automatically
    QVarLengthArray <int> m_intervalAccumulator;
    QVarLengthArray <float> m_dx, m_rampTo;
    QVarLengthArray <bool> m_ramp;
    QVarLengthArray <PitchFilter*> m_pitchFilter;
    QHash<int, int> m_scratchTimers;
};

#endif
