/**
* @file controllerengine.h
* @author Sean M. Pappalardo spappalardo@mixxx.org
* @date Sat Apr 30 2011
* @brief The script engine for use by a Controller.
*/

#ifndef CONTROLLERENGINE_H
#define CONTROLLERENGINE_H

#include <QTimerEvent>
#include <QFileSystemWatcher>
#include <QMessageBox>
#include <QtScript>

#include "bytearrayclass.h"
#include "preferences/usersettings.h"
#include "controllers/controllerpreset.h"
#include "controllers/softtakeover.h"
#include "util/alphabetafilter.h"
#include "util/duration.h"

// Forward declaration(s)
class Controller;
class ControlObjectScript;
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
        m_conn = conn;
    }
    const QString& readId() const { return m_conn.id; }
    Q_INVOKABLE void disconnect();

  private:
    ControllerEngineConnection m_conn;
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

    // Check whether a source file that was evaluated()'d has errors.
    bool hasErrors(const QString& filename);

    // Get the errors for a source file that was evaluated()'d
    const QStringList getErrors(const QString& filename);

    void setPopups(bool bPopups) {
        m_bPopups = bPopups;
    }

    // Wrap a snippet of JS code in an anonymous function
    QScriptValue wrapFunctionCode(const QString& codeSnippet, int numberOfArgs);

    // Look up registered script function prefixes
    const QList<QString>& getScriptFunctionPrefixes() { return m_scriptFunctionPrefixes; };

    // Disconnect a ControllerEngineConnection
    void disconnectControl(const ControllerEngineConnection conn);

  protected:
    Q_INVOKABLE double getValue(QString group, QString name);
    Q_INVOKABLE void setValue(QString group, QString name, double newValue);
    Q_INVOKABLE double getParameter(QString group, QString name);
    Q_INVOKABLE void setParameter(QString group, QString name, double newValue);
    Q_INVOKABLE double getParameterForValue(QString group, QString name, double value);
    Q_INVOKABLE void reset(QString group, QString name);
    Q_INVOKABLE double getDefaultValue(QString group, QString name);
    Q_INVOKABLE double getDefaultParameter(QString group, QString name);
    Q_INVOKABLE QScriptValue connectControl(QString group, QString name,
                                            QScriptValue function, bool disconnect = false);
    // Called indirectly by the objects returned by connectControl
    Q_INVOKABLE void trigger(QString group, QString name);
    Q_INVOKABLE void log(QString message);
    Q_INVOKABLE int beginTimer(int interval, QScriptValue scriptCode, bool oneShot = false);
    Q_INVOKABLE void stopTimer(int timerId);
    Q_INVOKABLE void scratchEnable(int deck, int intervalsPerRev, double rpm,
                                   double alpha, double beta, bool ramp = true);
    Q_INVOKABLE void scratchTick(int deck, int interval);
    Q_INVOKABLE void scratchDisable(int deck, bool ramp = true);
    Q_INVOKABLE bool isScratching(int deck);
    Q_INVOKABLE void softTakeover(QString group, QString name, bool set);
    Q_INVOKABLE void softTakeoverIgnoreNextValue(QString group, QString name);
    Q_INVOKABLE void brake(int deck, bool activate, double factor=0.9, double rate=1.0);
    Q_INVOKABLE void spinback(int deck, bool activate, double factor=1.8, double rate=-10.0);

    // Handler for timers that scripts set.
    virtual void timerEvent(QTimerEvent *event);

  public slots:
    // Evaluate a script file
    bool evaluate(const QString& filepath);

    // Execute a basic MIDI message callback.
    bool execute(QScriptValue function,
                 unsigned char channel,
                 unsigned char control,
                 unsigned char value,
                 unsigned char status,
                 const QString& group,
                 mixxx::Duration timestamp);

    // Execute a byte array callback.
    bool execute(QScriptValue function, const QByteArray data,
                 mixxx::Duration timestamp);

    // Evaluates all provided script files and returns true if no script errors
    // occurred while evaluating them.
    bool loadScriptFiles(const QList<QString>& scriptPaths,
                         const QList<ControllerPreset::ScriptFileInfo>& scripts);
    void initializeScripts(const QList<ControllerPreset::ScriptFileInfo>& scripts);
    void gracefulShutdown();
    void scriptHasChanged(const QString&);

  signals:
    void initialized();
    void resetController();

  private slots:
    void errorDialogButton(const QString& key, QMessageBox::StandardButton button);

  private:
    bool syntaxIsValid(const QString& scriptCode);
    bool evaluate(const QString& scriptName, QList<QString> scriptPaths);
    bool internalExecute(QScriptValue thisObject, const QString& scriptCode);
    bool internalExecute(QScriptValue thisObject, QScriptValue functionObject,
                         QScriptValueList arguments);
    void initializeScriptEngine();

    void scriptErrorDialog(const QString& detailedError);
    void generateScriptFunctions(const QString& code);
    // Stops and removes all timers (for shutdown).
    void stopAllTimers();

    void callFunctionOnObjects(QList<QString>, const QString&, QScriptValueList args = QScriptValueList());
    bool checkException();
    QScriptEngine *m_pEngine;

    ControlObjectScript* getControlObjectScript(const QString& group, const QString& name);

    // Scratching functions & variables
    void scratchProcess(int timerId);

    bool isDeckPlaying(const QString& group);
    double getDeckRate(const QString& group);

    Controller* m_pController;
    bool m_bPopups;
    QMultiHash<ConfigKey, ControllerEngineConnection> m_connectedControls;
    QList<QString> m_scriptFunctionPrefixes;
    QMap<QString, QStringList> m_scriptErrors;
    QHash<ConfigKey, ControlObjectScript*> m_controlCache;
    struct TimerInfo {
        QScriptValue callback;
        QScriptValue context;
        bool oneShot;
    };
    QHash<int, TimerInfo> m_timers;
    SoftTakeoverCtrl m_st;
    ByteArrayClass* m_pBaClass;
    // 256 (default) available virtual decks is enough I would think.
    //  If more are needed at run-time, these will move to the heap automatically
    QVarLengthArray<int> m_intervalAccumulator;
    QVarLengthArray<mixxx::Duration> m_lastMovement;
    QVarLengthArray<double> m_dx, m_rampTo, m_rampFactor;
    QVarLengthArray<bool> m_ramp, m_brakeActive;
    QVarLengthArray<AlphaBetaFilter*> m_scratchFilters;
    QHash<int, int> m_scratchTimers;
    QHash<QString, QScriptValue> m_scriptWrappedFunctionCache;
    // Filesystem watcher for script auto-reload
    QFileSystemWatcher m_scriptWatcher;
    QList<QString> m_lastScriptPaths;

    friend class ControllerEngineTest;
};

#endif
