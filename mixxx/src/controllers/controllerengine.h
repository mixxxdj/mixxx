/**
* @file controllerengine.h
* @author Sean M. Pappalardo spappalardo@mixxx.org
* @date Sat Apr 30 2011
* @brief The script engine for use by a Controller.
*/

#ifndef CONTROLLERENGINE_H
#define CONTROLLERENGINE_H

#include <QEvent>
#include <QtScript>
#include <QMessageBox>
#include <QFileSystemWatcher>

#include "configobject.h"
#include "controllers/pitchfilter.h"
#include "controllers/softtakeover.h"
#include "qtscript-bytearray/bytearrayclass.h"

// Forward declaration(s)
class Controller;
class ControlObjectThread;
class ControllerEngine;

// ControllerEngineConnection class for closure-compatible engine.connectControl
class ControllerEngineConnection {
  public:
    ConfigKey key;
    QString id;
    QScriptValue function;
    ControllerEngine *ce;
    QScriptValue context;
};

class ControllerEngineConnectionScriptValue : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString id READ readId)
    // We cannot expose ConfigKey directly since it's not a
    // QObject
    //Q_PROPERTY(ConfigKey key READ key)
    // There's little use in exposing the function...
    //Q_PROPERTY(QScriptValue function READ function)
  public:
    ControllerEngineConnectionScriptValue(ControllerEngineConnection conn) {
        this->conn = conn;
    }
    QString readId() const { return this->conn.id; }
    Q_INVOKABLE void disconnect();

  private:
   ControllerEngineConnection conn;
};

/* comparison function for ControllerEngineConnection */
inline bool operator==(const ControllerEngineConnection &c1, const ControllerEngineConnection &c2) {
    return c1.id == c2.id && c1.key.group == c2.key.group && c1.key.item == c2.key.item;
}

class ControllerEngine : public QObject {
    Q_OBJECT
  public:
    ControllerEngine(Controller* controller);
    virtual ~ControllerEngine();

    bool isReady();
    bool hasErrors(QString filename);
    const QStringList getErrors(QString filename);

    void setDebug(bool bDebug) {
        m_bDebug = bDebug;
    }

    void setPopups(bool bPopups) {
        m_bPopups = bPopups;
    }

    /** Resolve a function name to a QScriptValue. */
    QScriptValue resolveFunction(QString function, bool useCache) const;
    /** Look up registered script function prefixes */
    QList<QString>& getScriptFunctionPrefixes() { return m_scriptFunctionPrefixes; };
    /** Disconnect a ControllerEngineConnection */
    void disconnectControl(const ControllerEngineConnection conn);

  protected:
    Q_INVOKABLE double getValue(QString group, QString name);
    Q_INVOKABLE void setValue(QString group, QString name, double newValue);
    Q_INVOKABLE QScriptValue connectControl(QString group, QString name,
                                    QScriptValue function, bool disconnect = false);
    // Called indirectly by the objects returned by connectControl
    Q_INVOKABLE void trigger(QString group, QString name);
    Q_INVOKABLE void log(QString message);
    Q_INVOKABLE int beginTimer(int interval, QScriptValue scriptCode, bool oneShot = false);
    Q_INVOKABLE void stopTimer(int timerId);
    Q_INVOKABLE void scratchEnable(int deck, int intervalsPerRev, float rpm,
                                   float alpha, float beta, bool ramp = true);
    Q_INVOKABLE void scratchTick(int deck, int interval);
    Q_INVOKABLE void scratchDisable(int deck, bool ramp = true);
    Q_INVOKABLE bool isScratching(int deck);
    Q_INVOKABLE void softTakeover(QString group, QString name, bool set);
    Q_INVOKABLE void brake(int deck, bool activate, float factor=0.9, float rate=1.0);
    Q_INVOKABLE void spinback(int deck, bool activate, float factor=1.8, float rate=-10.0);

    // Handler for timers that scripts set.
    virtual void timerEvent(QTimerEvent *event);

  public slots:
    void slotValueChanged(double value);
    // Evaluate a script file
    bool evaluate(QString filepath);

    // Execute a particular function
    bool execute(QString function);
    // Execute a particular function with a list of arguments
    bool execute(QString function, QScriptValueList args);
    bool execute(QScriptValue function, QScriptValueList args);
    // Execute a particular function with a data string (e.g. a device ID)
    bool execute(QString function, QString data);
    // Execute a particular function with a list of arguments
    bool execute(QString function, const QByteArray data);
    bool execute(QScriptValue function, const QByteArray data);
    // Execute a particular function with a data buffer
    //TODO: redo this one
    //bool execute(QString function, const QByteArray data);
    void loadScriptFiles(QList<QString> scriptPaths,
                         QList<QString> scriptFileNames);
    void initializeScripts(const QList<QString> scriptFunctionPrefixes);
    void gracefulShutdown();
    void scriptHasChanged(QString);

  signals:
    void initialized();
    void resetController();

  private slots:
    void errorDialogButton(QString key, QMessageBox::StandardButton button);

  private:
    bool evaluate(QString scriptName, QList<QString> scriptPaths);
    bool internalExecute(QString scriptCode);
    bool internalExecute(QScriptValue thisObject, QString scriptCode);
    bool internalExecute(QScriptValue thisObject, QScriptValue functionObject);
    void initializeScriptEngine();

    void scriptErrorDialog(QString detailedError);
    void generateScriptFunctions(QString code);
    void stopAllTimers();

    void callFunctionOnObjects(QList<QString>, QString, QScriptValueList args = QScriptValueList());
    bool checkException();
    QScriptEngine *m_pEngine;

    ControlObjectThread* getControlObjectThread(QString group, QString name);

    // Scratching functions & variables
    void scratchProcess(int timerId);

    Controller* m_pController;
    bool m_bDebug;
    bool m_bPopups;
    QMultiHash<ConfigKey, ControllerEngineConnection> m_connectedControls;
    QList<QString> m_scriptFunctionPrefixes;
    QMap<QString,QStringList> m_scriptErrors;
    QHash<ConfigKey, ControlObjectThread*> m_controlCache;
    struct TimerInfo {
        QScriptValue callback;
        QScriptValue context;
        bool oneShot;
    };
    QHash<int, TimerInfo> m_timers;
    SoftTakeover m_st;
    ByteArrayClass *m_pBaClass;
    // 256 (default) available virtual decks is enough I would think.
    //  If more are needed at run-time, these will move to the heap automatically
    QVarLengthArray<int> m_intervalAccumulator, m_brakeKeylock;
    QVarLengthArray<uint> m_lastMovement;
    QVarLengthArray<float> m_dx, m_rampTo, m_rampFactor;
    QVarLengthArray<bool> m_ramp, m_brakeActive;
    QVarLengthArray<PitchFilter*> m_pitchFilter;
    QHash<int, int> m_scratchTimers;
    mutable QHash<QString, QScriptValue> m_scriptValueCache;
    // Filesystem watcher for script auto-reload
    QFileSystemWatcher m_scriptWatcher;
    QList<QString> m_lastScriptPaths;
};

#endif
