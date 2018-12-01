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
#include <QJSEngine>
#include <QJSValue>

#include "preferences/usersettings.h"
#include "controllers/controllerpreset.h"
#include "controllers/softtakeover.h"
#include "util/alphabetafilter.h"
#include "util/duration.h"

// Forward declaration(s)
class Controller;
class ControlObjectScript;
class ControllerEngine;
class ControllerEngineJSProxy;
class EvaluationException;

// ScriptConnection represents a connection between
// a ControlObject and a script callback function that gets executed when
// the value of the ControlObject changes.
class ScriptConnection {
  public:
    ConfigKey key;
    QUuid id;
    QJSValue callback;
    ControllerEngine *controllerEngine;

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
  public:
    ScriptConnectionInvokableWrapper(ScriptConnection conn) {
        m_scriptConnection = conn;
        m_idString = conn.id.toString();
    }
    const QString& readId() const { return m_idString; }
    Q_INVOKABLE void disconnect();
    Q_INVOKABLE void trigger();

  private:
    ScriptConnection m_scriptConnection;
    QString m_idString;
};

class ControllerEngine : public QObject {
    Q_OBJECT
  public:
    ControllerEngine(Controller* controller);
    virtual ~ControllerEngine();

    void setPopups(bool bPopups) {
        m_bPopups = bPopups;
    }

    bool executeFunction(QJSValue functionObject, QJSValueList arguments);
    bool executeFunction(QJSValue functionObject, const QByteArray data);

    // Wrap a snippet of JS code in an anonymous function
    // Throws EvaluationException and NullEngineException.
    QJSValue wrapFunctionCode(const QString& codeSnippet, int numberOfArgs);

    // Look up registered script function prefixes
    const QList<QString>& getScriptFunctionPrefixes() { return m_scriptFunctionPrefixes; };

    // Disconnect a ScriptConnection
    void removeScriptConnection(const ScriptConnection conn);
    void triggerScriptConnection(const ScriptConnection conn);

  protected:
    double getValue(QString group, QString name);
    void setValue(QString group, QString name, double newValue);
    double getParameter(QString group, QString name);
    void setParameter(QString group, QString name, double newValue);
    double getParameterForValue(QString group, QString name, double value);
    void reset(QString group, QString name);
    double getDefaultValue(QString group, QString name);
    double getDefaultParameter(QString group, QString name);
    QJSValue makeConnection(QString group, QString name,
                                            const QJSValue callback);
    // DEPRECATED: Use makeConnection instead.
    QJSValue connectControl(QString group, QString name,
                                            const QJSValue passedCallback,
                                            bool disconnect = false);
    // Called indirectly by the objects returned by connectControl
    void trigger(QString group, QString name);
    void log(QString message);
    int beginTimer(int interval, QJSValue scriptCode, bool oneShot = false);
    void stopTimer(int timerId);
    void scratchEnable(int deck, int intervalsPerRev, double rpm,
                       double alpha, double beta, bool ramp = true);
    void scratchTick(int deck, int interval);
    void scratchDisable(int deck, bool ramp = true);
    bool isScratching(int deck);
    void softTakeover(QString group, QString name, bool set);
    void softTakeoverIgnoreNextValue(QString group, QString name);
    void brake(int deck, bool activate, double factor=1.0, double rate=1.0);
    void spinback(int deck, bool activate, double factor=1.8, double rate=-10.0);
    void softStart(int deck, bool activate, double factor=1.0);

    // Handler for timers that scripts set.
    virtual void timerEvent(QTimerEvent *event);

  public slots:
    // Evaluates all provided script files and returns true if no script errors
    // occurred while evaluating them.
    bool loadScriptFiles(const QList<QString>& scriptPaths,
                         const QList<ControllerPreset::ScriptFileInfo>& scripts);
    void initializeScripts(const QList<ControllerPreset::ScriptFileInfo>& scripts);
    void gracefulShutdown();
    void scriptHasChanged(const QString&);

  signals:
    void initialized();

  private slots:
    void errorDialogButton(const QString& key, QMessageBox::StandardButton button);

  private:
    bool evaluateScriptFile(const QString& scriptName, QList<QString> scriptPaths);
    bool internalExecute(QJSValue thisObject, const QString& scriptCode);
    bool internalExecute(const QString& scriptCode);
    bool internalExecute(QJSValue thisObject, QJSValue functionObject,
                         QJSValueList arguments);
    void initializeScriptEngine();
    void reloadScripts();

    void scriptErrorDialog(const QString& detailedError);
    void generateScriptFunctions(const QString& code);
    // Stops and removes all timers (for shutdown).
    void stopAllTimers();

    void callFunctionOnObjects(QList<QString>, const QString&, QJSValueList args = QJSValueList());
    // Convert a byteArray to a JS typed array over an ArrayBuffer
    QJSValue byteArrayToScriptValue(const QByteArray byteArray);
    // Throws EvaluationException and NullEngineException.
    QJSValue evaluateCodeString(const QString& program, const QString& fileName = QString(),
    		int lineNumber = 1);

    // Shows a UI dialog notifying of a script evaluation error.
    // Precondition: QJSValue.isError() == true
    void showScriptExceptionDialog(QJSValue evaluationResult);
    QJSEngine *m_pScriptEngine;

    ControlObjectScript* getControlObjectScript(const QString& group, const QString& name);

    // Scratching functions & variables
    void scratchProcess(int timerId);

    bool isDeckPlaying(const QString& group);
    double getDeckRate(const QString& group);

    Controller* m_pController;
    bool m_bPopups;
    QList<QString> m_scriptFunctionPrefixes;
    QHash<ConfigKey, ControlObjectScript*> m_controlCache;
    struct TimerInfo {
        QJSValue callback;
        bool oneShot;
    };
    QHash<int, TimerInfo> m_timers;
    SoftTakeoverCtrl m_st;
    // 256 (default) available virtual decks is enough I would think.
    //  If more are needed at run-time, these will move to the heap automatically
    QVarLengthArray<int> m_intervalAccumulator;
    QVarLengthArray<mixxx::Duration> m_lastMovement;
    QVarLengthArray<double> m_dx, m_rampTo, m_rampFactor;
    QVarLengthArray<bool> m_ramp, m_brakeActive, m_softStartActive;
    QVarLengthArray<AlphaBetaFilter*> m_scratchFilters;
    QHash<int, int> m_scratchTimers;
    QHash<QString, QJSValue> m_scriptWrappedFunctionCache;
    QJSValue m_byteArrayToScriptValueJSFunction;
    // Filesystem watcher for script auto-reload
    QFileSystemWatcher m_scriptWatcher;
    QList<QString> m_lastScriptPaths;

    friend class ControllerEngineJSProxy;
    friend class ControllerEngineTest;
};

#endif
