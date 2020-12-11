#pragma once

#include <QFileSystemWatcher>
#include <QJSEngine>
#include <QJSValue>
#include <QMessageBox>
#include <QTimerEvent>

#include "controllers/controllerpreset.h"
#include "controllers/softtakeover.h"
#include "preferences/usersettings.h"
#include "util/alphabetafilter.h"
#include "util/duration.h"

class Controller;
class ControlObjectScript;
class ControllerEngine;
class ControllerEngineJSProxy;
class EvaluationException;
class ScriptConnection;

class ControllerEngine : public QObject {
    Q_OBJECT
  public:
    ControllerEngine(Controller* controller);
    virtual ~ControllerEngine();

    void handleInput(const QByteArray& data, mixxx::Duration timestamp);

    bool executeFunction(QJSValue functionObject, const QJSValueList& arguments);
    bool executeFunction(const QJSValue& functionObject, const QByteArray& data);

    /// Wrap a string of JS code in an anonymous function. This allows any JS
    /// string that evaluates to a function to be used in MIDI mapping XML files
    /// and ensures the function is executed with the correct 'this' object.
    QJSValue wrapFunctionCode(const QString& codeSnippet, int numberOfArgs);

    /// Look up registered script function prefixes
    const QList<QString>& getScriptFunctionPrefixes() {
        return m_scriptFunctionPrefixes;
    };

    /// Shows a UI dialog notifying of a script evaluation error.
    /// Precondition: QJSValue.isError() == true
    void showScriptExceptionDialog(const QJSValue& evaluationResult, bool bFatal = false);

    bool removeScriptConnection(const ScriptConnection& conn);
    /// Execute a ScriptConnection's JS callback
    void triggerScriptConnection(const ScriptConnection& conn);

    inline void setTesting(bool testing) {
        m_bTesting = testing;
    };

  protected:
    double getValue(const QString& group, const QString& name);
    void setValue(const QString& group, const QString& name, double newValue);
    double getParameter(const QString& group, const QString& name);
    void setParameter(const QString& group, const QString& name, double newValue);
    double getParameterForValue(const QString& group, const QString& name, double value);
    void reset(const QString& group, const QString& name);
    double getDefaultValue(const QString& group, const QString& name);
    double getDefaultParameter(const QString& group, const QString& name);
    /// Connect a ControlObject's valueChanged() signal to a script callback function
    /// Returns to the script a ScriptConnectionJSProxy
    QJSValue makeConnection(const QString& group, const QString& name, const QJSValue& callback);
    /// DEPRECATED: Use makeConnection instead.
    QJSValue connectControl(const QString& group,
            const QString& name,
            const QJSValue& passedCallback,
            bool disconnect = false);
    /// Execute callbacks for all ScriptConnections connected to a ControlObject
    /// DEPRECATED: Use ScriptConnectionJSProxy::trigger instead.
    void trigger(const QString& group, const QString& name);
    void log(const QString& message);
    /// Returns a timer ID to the script
    int beginTimer(int intervalMillis, QJSValue scriptCode, bool oneShot = false);
    void stopTimer(int timerId);

    /// [En/dis]able soft-takeover status for a particular ControlObject
    void softTakeover(const QString& group, const QString& name, bool set);
    /// Ignores the next value for the given ControlObject. This should be called
    /// before or after an absolute physical control (slider or knob with hard limits)
    /// is changed to operate on a different ControlObject, allowing it to sync up to the
    /// soft-takeover state without an abrupt jump.
    void softTakeoverIgnoreNextValue(const QString& group, const QString& name);

    void scratchEnable(
            int deck,
            int intervalsPerRev,
            double rpm,
            double alpha,
            double beta,
            bool ramp = true);
    /// Accumulates ticks of the controller wheel
    void scratchTick(int deck, int interval);
    void scratchDisable(int deck, bool ramp = true);
    bool isScratching(int deck);
    void brake(int deck, bool activate, double factor = 1.0, double rate = 1.0);
    void spinback(int deck, bool activate, double factor = 1.8, double rate = -10.0);
    void softStart(int deck, bool activate, double factor = 1.0);

    /// Handler for timers that scripts set.
    virtual void timerEvent(QTimerEvent* event);

  public slots:
    void loadModule(const QFileInfo& moduleFileInfo);
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
    /// Stops and removes all timers (for shutdown).
    void stopAllTimers();

    bool callFunctionOnObjects(
            const QList<QString>&,
            const QString&,
            const QJSValueList& args = QJSValueList(),
            bool bFatalError = false);
    /// Convert a byteArray to a JS typed array over an ArrayBuffer
    QJSValue byteArrayToScriptValue(const QByteArray& byteArray);
    QJSValue evaluateCodeString(const QString& program, const QString& fileName = QString(), int lineNumber = 1);

    void throwJSError(const QString& message);

    bool m_bDisplayingExceptionDialog;
    QJSEngine* m_pScriptEngine;

    ControlObjectScript* getControlObjectScript(const QString& group, const QString& name);

    // Scratching functions & variables

    /// Applies the accumulated movement to the track speed
    void scratchProcess(int timerId);

    bool isDeckPlaying(const QString& group);
    double getDeckRate(const QString& group);

    Controller* m_pController;
    QJSValue m_handleInputFunction;
    QJSValue m_shutdownFunction;
    QList<QString> m_scriptFunctionPrefixes;
    QHash<ConfigKey, ControlObjectScript*> m_controlCache;
    struct TimerInfo {
        QJSValue callback;
        bool oneShot;
    };
    QHash<int, TimerInfo> m_timers;
    SoftTakeoverCtrl m_st;
    // 256 (default) available virtual decks is enough I would think.
    // If more are needed at run-time, these will move to the heap automatically
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
    QFileInfo m_moduleFileInfo;
    QList<ControllerPreset::ScriptFileInfo> m_lastScriptFiles;

    bool m_bTesting;

    friend class ScriptConnection;
    friend class ControllerEngineJSProxy;
    friend class ColorJSProxy;
    friend class ColorMapperJSProxy;
    friend class ControllerEngineTest;
};
