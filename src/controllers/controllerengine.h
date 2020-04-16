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
#include "util/memory.h"

// Forward declaration(s)
class Controller;
class ControlObjectScript;
class ControllerEngine;

// ScriptConnection represents a connection between
// a ControlObject and a script callback function that gets executed when
// the value of the ControlObject changes.
class ScriptConnection {
  public:
    ConfigKey key;
    QUuid id;
    QScriptValue callback;
    ControllerEngine *controllerEngine;
    QScriptValue context;

    void executeCallback(double value) const;

    // Required for various QList methods and iteration to work.
    inline bool operator==(const ScriptConnection& other) const {
        return id == other.id;
    }
    inline bool operator!=(const ScriptConnection& other) const {
        return !(*this == other);
    }
};

// ScriptConnectionInvokableWrapper is a class providing scripts
// with an interface to ScriptConnection.
class ScriptConnectionInvokableWrapper : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString id READ readId)
    // We cannot expose ConfigKey directly since it's not a
    // QObject
    //Q_PROPERTY(ConfigKey key READ key)
    // There's little use in exposing the function...
    //Q_PROPERTY(QScriptValue function READ function)
    Q_PROPERTY(bool isConnected READ readIsConnected)
  public:
    ScriptConnectionInvokableWrapper(ScriptConnection conn) {
        m_scriptConnection = conn;
        m_idString = conn.id.toString();
        m_isConnected = true;
    }
    const QString& readId() const { return m_idString; }
    bool readIsConnected() const { return m_isConnected; }
    Q_INVOKABLE bool disconnect();
    Q_INVOKABLE void trigger();

  private:
    ScriptConnection m_scriptConnection;
    QString m_idString;
    bool m_isConnected;
};

class ControllerEngine : public QObject {
    Q_OBJECT
  public:
    ControllerEngine(Controller* controller, UserSettingsPointer pConfig);
    virtual ~ControllerEngine();

    bool isReady();

    // Check whether a source file that was evaluated()'d has errors.
    bool hasErrors(const QString& filename);

    void setPopups(bool bPopups) {
        m_bPopups = bPopups;
    }

    // Wrap a snippet of JS code in an anonymous function
    QScriptValue wrapFunctionCode(const QString& codeSnippet, int numberOfArgs);
    QScriptValue getThisObjectInFunctionCall();

    // Look up registered script function prefixes
    const QList<QString>& getScriptFunctionPrefixes() { return m_scriptFunctionPrefixes; };

    // Disconnect a ScriptConnection
    bool removeScriptConnection(const ScriptConnection conn);
    void triggerScriptConnection(const ScriptConnection conn);

  protected:
    Q_INVOKABLE double getValue(QString group, QString name);
    Q_INVOKABLE void setValue(QString group, QString name, double newValue);
    Q_INVOKABLE double getParameter(QString group, QString name);
    Q_INVOKABLE void setParameter(QString group, QString name, double newValue);
    Q_INVOKABLE double getParameterForValue(QString group, QString name, double value);
    Q_INVOKABLE void reset(QString group, QString name);
    Q_INVOKABLE double getDefaultValue(QString group, QString name);
    Q_INVOKABLE double getDefaultParameter(QString group, QString name);
    Q_INVOKABLE QScriptValue makeConnection(QString group, QString name,
                                            const QScriptValue callback);
    // DEPRECATED: Use makeConnection instead.
    Q_INVOKABLE QScriptValue connectControl(QString group, QString name,
                                            const QScriptValue passedCallback,
                                            bool disconnect = false);
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
    Q_INVOKABLE void brake(int deck, bool activate, double factor=1.0, double rate=1.0);
    Q_INVOKABLE void spinback(int deck, bool activate, double factor=1.8, double rate=-10.0);
    Q_INVOKABLE void softStart(int deck, bool activate, double factor=1.0);

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
    bool loadScriptFiles(const QList<ControllerPreset::ScriptFileInfo>& scripts);
    void initializeScripts(const QList<ControllerPreset::ScriptFileInfo>& scripts);
    void gracefulShutdown();
    void scriptHasChanged(const QString&);

  signals:
    void resetController();

  private slots:
    void errorDialogButton(const QString& key, QMessageBox::StandardButton button);

  private:
    bool syntaxIsValid(const QString& scriptCode, const QString& filename = QString());
    bool evaluate(const QFileInfo& scriptFile);
    bool internalExecute(QScriptValue thisObject, const QString& scriptCode);
    bool internalExecute(QScriptValue thisObject, QScriptValue functionObject,
                         QScriptValueList arguments);
    void initializeScriptEngine();
    void uninitializeScriptEngine();

    void scriptErrorDialog(const QString& detailedError, const QString& key, bool bFatal = false);
    void generateScriptFunctions(const QString& code);
    // Stops and removes all timers (for shutdown).
    void stopAllTimers();

    void callFunctionOnObjects(QList<QString>, const QString&, QScriptValueList args = QScriptValueList());
    bool checkException(bool bFatal = false);
    QScriptEngine *m_pEngine;

    ControlObjectScript* getControlObjectScript(const QString& group, const QString& name);

    // Scratching functions & variables
    void scratchProcess(int timerId);

    bool isDeckPlaying(const QString& group);
    double getDeckRate(const QString& group);

    Controller* m_pController;
    const UserSettingsPointer m_pConfig;
    bool m_bPopups;
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
    QVarLengthArray<bool> m_ramp, m_brakeActive, m_softStartActive;
    QVarLengthArray<AlphaBetaFilter*> m_scratchFilters;
    QHash<int, int> m_scratchTimers;
    QHash<QString, QScriptValue> m_scriptWrappedFunctionCache;
    // Filesystem watcher for script auto-reload
    QFileSystemWatcher m_scriptWatcher;
    QList<ControllerPreset::ScriptFileInfo> m_lastScriptFiles;

    friend class ControllerEngineTest;
};

#endif
