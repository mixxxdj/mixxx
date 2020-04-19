/**
* @file controllerengine.h
* @author Sean M. Pappalardo spappalardo@mixxx.org
* @date Sat Apr 30 2011
* @brief The script engine for use by a Controller.
*/

#ifndef CONTROLLERENGINE_H
#define CONTROLLERENGINE_H

#include <QFileSystemWatcher>
#include <QJSEngine>
#include <QJSValue>
#include <QMessageBox>
#include <QTimerEvent>
#include <QtScript>

#include "controllers/controllerpreset.h"
#include "controllers/softtakeover.h"
#include "preferences/usersettings.h"
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
    ControllerEngine* controllerEngine;

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
    Q_PROPERTY(bool isConnected READ readIsConnected)
  public:
    ScriptConnectionInvokableWrapper(ScriptConnection conn) {
        m_scriptConnection = conn;
        m_idString = conn.id.toString();
        m_isConnected = true;
    }
    const QString& readId() const {
        return m_idString;
    }
    bool readIsConnected() const {
        return m_isConnected;
    }
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
    ControllerEngine(Controller* controller);
    virtual ~ControllerEngine();

    // The controller engine version is used to check compatibility of presets.
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    // Qt >= 5.12.0 supports ECMAScript 7 (2016)
    static const int version = 2;
#else
    // Qt < 5.12.0 supports ECMAScript 5 (2009)
    static const int version = 1;
#endif

    // Execute a JS function in the engine
    bool executeFunction(QJSValue functionObject, QJSValueList arguments);
    bool executeFunction(QJSValue functionObject, const QByteArray data);

    // Wrap a snippet of JS code in an anonymous function
    QJSValue wrapFunctionCode(const QString& codeSnippet, int numberOfArgs);

    // Look up registered script function prefixes
    const QList<QString>& getScriptFunctionPrefixes() {
        return m_scriptFunctionPrefixes;
    };

    // Disconnect a ScriptConnection
    bool removeScriptConnection(const ScriptConnection conn);
    void triggerScriptConnection(const ScriptConnection conn);

    inline void setTesting(bool testing) {
        m_bTesting = testing;
    };

  protected:
    double getValue(QString group, QString name);
    void setValue(QString group, QString name, double newValue);
    double getParameter(QString group, QString name);
    void setParameter(QString group, QString name, double newValue);
    double getParameterForValue(QString group, QString name, double value);
    void reset(QString group, QString name);
    double getDefaultValue(QString group, QString name);
    double getDefaultParameter(QString group, QString name);
    QJSValue makeConnection(QString group, QString name, const QJSValue callback);
    // DEPRECATED: Use makeConnection instead.
    QJSValue connectControl(QString group, QString name, const QJSValue passedCallback, bool disconnect = false);
    // Called indirectly by the objects returned by connectControl
    void trigger(QString group, QString name);
    void log(QString message);
    int beginTimer(int interval, QJSValue scriptCode, bool oneShot = false);
    void stopTimer(int timerId);
    void scratchEnable(int deck, int intervalsPerRev, double rpm, double alpha, double beta, bool ramp = true);
    void scratchTick(int deck, int interval);
    void scratchDisable(int deck, bool ramp = true);
    bool isScratching(int deck);
    void softTakeover(QString group, QString name, bool set);
    void softTakeoverIgnoreNextValue(QString group, QString name);
    void brake(int deck, bool activate, double factor = 1.0, double rate = 1.0);
    void spinback(int deck, bool activate, double factor = 1.8, double rate = -10.0);
    void softStart(int deck, bool activate, double factor = 1.0);

    // Handler for timers that scripts set.
    virtual void timerEvent(QTimerEvent* event);

  public slots:
    // Evaluates all provided script files and returns true if no script errors
    // occurred while evaluating them.
    bool loadScriptFiles(const QList<ControllerPreset::ScriptFileInfo>& scripts);
    void initializeScripts(const QList<ControllerPreset::ScriptFileInfo>& scripts);
    void gracefulShutdown();
    void scriptHasChanged(const QString&);

  private slots:
    void errorDialogButton(const QString& key, QMessageBox::StandardButton button);

  private:
    bool evaluateScriptFile(const QFileInfo& scriptFile);
    void initializeScriptEngine();
    void uninitializeScriptEngine();
    void reloadScripts();

    void scriptErrorDialog(const QString& detailedError, const QString& key, bool bFatal = false);
    void generateScriptFunctions(const QString& code);
    // Stops and removes all timers (for shutdown).
    void stopAllTimers();

    bool callFunctionOnObjects(QList<QString>,
            const QString&,
            QJSValueList args = QJSValueList(),
            bool bFatalError = false);
    // Convert a byteArray to a JS typed array over an ArrayBuffer
    QJSValue byteArrayToScriptValue(const QByteArray byteArray);
    QJSValue evaluateCodeString(const QString& program, const QString& fileName = QString(), int lineNumber = 1);

    void throwJSError(const QString& message);

    // Shows a UI dialog notifying of a script evaluation error.
    // Precondition: QJSValue.isError() == true
    void showScriptExceptionDialog(QJSValue evaluationResult, bool bFatal = false);
    bool m_bDisplayingExceptionDialog;
    QJSEngine* m_pScriptEngine;

    ControlObjectScript* getControlObjectScript(const QString& group, const QString& name);

    // Scratching functions & variables
    void scratchProcess(int timerId);

    bool isDeckPlaying(const QString& group);
    double getDeckRate(const QString& group);

    Controller* m_pController;
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
    QList<ControllerPreset::ScriptFileInfo> m_lastScriptFiles;

    bool m_bTesting;

    friend class ScriptConnection;
    friend class ControllerEngineJSProxy;
    friend class ColorJSProxy;
    friend class ColorMapperJSProxy;
    friend class ControllerEngineTest;
};

#endif
