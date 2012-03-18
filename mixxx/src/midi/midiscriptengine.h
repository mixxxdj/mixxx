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
#include "softtakeover.h"

class MidiDevice;

//Forward declaration(s)
class ControlObjectThread;
class MidiScriptEngine;

class MidiScriptEngineControllerConnection {
  public:
    ConfigKey key;
    QString id;
    QScriptValue function;
    MidiScriptEngine *mse;
    QScriptValue context;
};

class MidiScriptEngineControllerConnectionScriptValueProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString id READ readId)
    // We cannot expose ConfigKey directly since it's not a
    // QObject
    //Q_PROPERTY(ConfigKey key READ key)
    // There's little use in exposing the function...
    //Q_PROPERTY(QScriptValue function READ function)
  public:
    MidiScriptEngineControllerConnectionScriptValueProxy(MidiScriptEngineControllerConnection conn) {
        this->conn = conn;
    }
    QString readId() const { return this->conn.id; }
    Q_INVOKABLE void disconnect();
    
  private:
    MidiScriptEngineControllerConnection conn;
};

/* comparison function for MidiScriptEngineControllerConnection; Used by a QHash in ControlObject */
inline bool operator==(const MidiScriptEngineControllerConnection &c1, const MidiScriptEngineControllerConnection &c2) {
    return c1.id == c2.id && c1.key.group == c2.key.group && c1.key.item == c2.key.item;
}

class MidiScriptEngine : public QThread {
    Q_OBJECT
  public:
    MidiScriptEngine(MidiDevice* midiDevice);
    virtual ~MidiScriptEngine();

    bool isReady();
    bool hasErrors(QString filename);
    const QStringList getErrors(QString filename);

    void setMidiDebug(bool bDebug) {
        m_midiDebug = bDebug;
    }

    void setMidiPopups(bool bPopups) {
        m_midiPopups = bPopups;
    }

    bool isLoaded() {
        return m_isLoaded;
    }
    
    // Resolve a function name to a QScriptValue.
    // Also resolves calls prefixed with an object name.
    QScriptValue resolveFunction(QString function);
    
    /* DO NOT CALL DIRECTLY,
       SHOULD ONLY BE CALLED FROM WITHIN SCRIPTS */
    Q_INVOKABLE double getValue(QString group, QString name);
    Q_INVOKABLE void setValue(QString group, QString name, double newValue);
    Q_INVOKABLE QScriptValue connectControl(QString group, QString name,
                                    QScriptValue function, bool disconnect = false);
    // Called indirectly by the objects returned by connectControl
    void disconnectControl(const MidiScriptEngineControllerConnection conn);
    Q_INVOKABLE void trigger(QString group, QString name);
    Q_INVOKABLE void log(QString message);
    Q_INVOKABLE int beginTimer(int interval, QScriptValue scriptCode, bool oneShot = false);
    Q_INVOKABLE void stopTimer(int timerId);
    Q_INVOKABLE void scratchEnable(int deck, int intervalsPerRev, float rpm, float alpha, float beta);
    Q_INVOKABLE void scratchTick(int deck, int interval);
    Q_INVOKABLE void scratchDisable(int deck);
    Q_INVOKABLE void softTakeover(QString group, QString name, bool set);

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
    bool execute(QScriptValue function, char channel,
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
    bool safeEvaluate(QString scriptName);
    //bool internalExecute(QString scriptCode);
    //bool safeEvaluate(QString scriptName, QList<QString> scriptPaths);
    bool internalExecute(QScriptValue thisObject, QString scriptCode);
    bool safeExecute(QString function);
    bool safeExecute(QString function, QString data);
    bool safeExecute(QString function, const unsigned char data[], unsigned int length);
    bool safeExecute(QString function, char channel,
                     char control, char value, MidiStatusByte status, QString group);
    bool safeExecute(QScriptValue thisObject, QScriptValue functionObject);
    bool safeExecute(QScriptValue functionObject, char channel, char control, char value,
                        MidiStatusByte status, QString group);
    void initializeScriptEngine();

    void scriptErrorDialog(QString detailedError);
    bool checkException();
    void stopAllTimers();

	void callFunctionOnObjects(QList<QString>, QString, QScriptValueList args = QScriptValueList());

    ControlObjectThread* getControlObjectThread(QString group, QString name);

    MidiDevice* m_pMidiDevice;
    bool m_midiDebug;
    bool m_midiPopups;
    QMultiHash<ConfigKey, MidiScriptEngineControllerConnection> m_connectedControls;
    QScriptEngine *m_pEngine;
    QStringList m_scriptFunctions;
    QMap<QString,QStringList> m_scriptErrors;
    QMutex m_scriptEngineLock;
    QHash<ConfigKey, ControlObjectThread*> m_controlCache;
    struct TimerInfo {
        QScriptValue callback;
        QScriptValue context;
        bool oneShot;
    };
    QHash<int, TimerInfo> m_timers;
    SoftTakeover m_st;

    // Scratching functions & variables
    void scratchProcess(int timerId);

    // 256 (default) available virtual decks is enough I would think.
    //  If more are needed at run-time, these will move to the heap automatically
    QVarLengthArray <int> m_intervalAccumulator;
    QVarLengthArray <float> m_dx, m_rampTo;
    QVarLengthArray <bool> m_ramp;
    QVarLengthArray <PitchFilter*> m_pitchFilter;
    QHash<int, int> m_scratchTimers;
    bool m_isLoaded;
};

#endif
