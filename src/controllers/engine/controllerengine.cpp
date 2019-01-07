/***************************************************************************
                          controllerengine.cpp  -  description
                          -------------------
    begin                : Sat Apr 30 2011
    copyright            : (C) 2011 by Sean M. Pappalardo
    email                : spappalardo@mixxx.org
 ***************************************************************************/

#include "controllers/engine/controllerengine.h"

#include "controllers/engine/controllerenginejsproxy.h"
#include "controllers/controller.h"
#include "controllers/controllerdebug.h"
#include "control/controlobject.h"
#include "control/controlobjectscript.h"
#include "errordialoghandler.h"
#include "mixer/playermanager.h"
// to tell the msvs compiler about `isnan`
#include "util/math.h"
#include "util/time.h"

// Used for id's inside controlConnection objects
// (closure compatible version of connectControl)
#include <QUuid>

const int kDecks = 16;

// Use 1ms for the Alpha-Beta dt. We're assuming the OS actually gives us a 1ms
// timer.
const int kScratchTimerMs = 1;
const double kAlphaBetaDt = kScratchTimerMs / 1000.0;

ControllerEngine::ControllerEngine(Controller* controller)
        : m_bDisplayingExceptionDialog(false),
          m_pScriptEngine(nullptr),
          m_pController(controller),
          m_bTesting(false) {
    // Handle error dialog buttons
    qRegisterMetaType<QMessageBox::StandardButton>("QMessageBox::StandardButton");

    // Pre-allocate arrays for average number of virtual decks
    m_intervalAccumulator.resize(kDecks);
    m_lastMovement.resize(kDecks);
    m_dx.resize(kDecks);
    m_rampTo.resize(kDecks);
    m_ramp.resize(kDecks);
    m_scratchFilters.resize(kDecks);
    m_rampFactor.resize(kDecks);
    m_brakeActive.resize(kDecks);
    m_softStartActive.resize(kDecks);
    // Initialize arrays used for testing and pointers
    for (int i = 0; i < kDecks; ++i) {
        m_dx[i] = 0.0;
        m_scratchFilters[i] = new AlphaBetaFilter();
        m_ramp[i] = false;
    }

    initializeScriptEngine();
}

ControllerEngine::~ControllerEngine() {
    // Clean up
    for (int i = 0; i < kDecks; ++i) {
        delete m_scratchFilters[i];
        m_scratchFilters[i] = nullptr;
    }

    // Delete the script engine, first clearing the pointer so that
    // other threads will not get the dead pointer after we delete it.
    if (m_pScriptEngine != nullptr) {
        QJSEngine *engine = m_pScriptEngine;
        m_pScriptEngine = nullptr;
        engine->deleteLater();
    }
}

/* -------- ------------------------------------------------------
Purpose: Calls the same method on a list of JS Objects
Input:   -
Output:  -
-------- ------------------------------------------------------ */
void ControllerEngine::callFunctionOnObjects(QList<QString> scriptFunctionPrefixes,
                                             const QString& function, QJSValueList args) {
    const QJSValue global = m_pScriptEngine->globalObject();

    for (const QString& prefixName : scriptFunctionPrefixes) {
        QJSValue prefix = global.property(prefixName);
        if (!prefix.isObject()) {
            qWarning() << "ControllerEngine: No" << prefixName << "object in script";
            continue;
        }

        QJSValue init = prefix.property(function);
        if (!init.isCallable()) {
            qWarning() << "ControllerEngine:" << prefixName << "has no" << function << " method";
            continue;
        }
        controllerDebug("ControllerEngine: Executing" << prefixName << "." << function);
        QJSValue result = init.callWithInstance(prefix, args);
        if (result.isError()) {
            showScriptExceptionDialog(result);
        }
    }
}

QJSValue ControllerEngine::byteArrayToScriptValue(const QByteArray byteArray) {
    // The QJSEngine converts the QByteArray to an ArrayBuffer object.
    QJSValue arrayBuffer = m_pScriptEngine->toScriptValue(byteArray);
    // We convert the ArrayBuffer to a Uint8 typed array so we can access its bytes
    // with the [] operator.
    QJSValueList args;
    args << arrayBuffer;
    QJSValue result = m_byteArrayToScriptValueJSFunction.call(args);
    if (result.isError()) {
        showScriptExceptionDialog(result);
    }
    return result;
}

/* ------------------------------------------------------------------
Purpose: Turn a snippet of JS into a QJSValue function.
         Wrapping it in an anonymous function allows any JS that
         evaluates to a function to be used in MIDI mapping XML files
         and ensures the function is executed with the correct
         'this' object.
Input:   QString snippet of JS that evaluates to a function,
         int number of arguments that the function takes
Output:  QJSValue of JS snippet wrapped in an anonymous function
------------------------------------------------------------------- */
QJSValue ControllerEngine::wrapFunctionCode(const QString& codeSnippet,
                                                int numberOfArgs) {
    QJSValue wrappedFunction;

    auto i = m_scriptWrappedFunctionCache.constFind(codeSnippet);
    if (i != m_scriptWrappedFunctionCache.constEnd()) {
        wrappedFunction = i.value();
    } else {
        QStringList wrapperArgList;
        for (int i = 1; i <= numberOfArgs; i++) {
            wrapperArgList << QString("arg%1").arg(i);
        }
        QString wrapperArgs = wrapperArgList.join(",");
        QString wrappedCode = "(function (" + wrapperArgs + ") { (" +
                                codeSnippet + ")(" + wrapperArgs + "); })";

        wrappedFunction = evaluateCodeString(wrappedCode);
        if (wrappedFunction.isError()) {
            showScriptExceptionDialog(wrappedFunction);
        }
        m_scriptWrappedFunctionCache[codeSnippet] = wrappedFunction;
    }
    return wrappedFunction;
}

/* -------- ------------------------------------------------------
Purpose: Shuts down scripts in an orderly fashion
            (stops timers then executes shutdown functions)
Input:   -
Output:  -
-------- ------------------------------------------------------ */
void ControllerEngine::gracefulShutdown() {
    qDebug() << "ControllerEngine shutting down...";

    // Stop all timers
    stopAllTimers();

    // Call each script's shutdown function if it exists
    callFunctionOnObjects(m_scriptFunctionPrefixes, "shutdown");

    // Prevents leaving decks in an unstable state
    //  if the controller is shut down while scratching
    QHashIterator<int, int> i(m_scratchTimers);
    while (i.hasNext()) {
        i.next();
        qDebug() << "Aborting scratching on deck" << i.value();
        // Clear scratch2_enable. PlayerManager::groupForDeck is 0-indexed.
        QString group = PlayerManager::groupForDeck(i.value() - 1);
        ControlObjectScript* pScratch2Enable =
                getControlObjectScript(group, "scratch2_enable");
        if (pScratch2Enable != nullptr) {
            pScratch2Enable->set(0);
        }
    }

    // Clear the cache of function wrappers
    m_scriptWrappedFunctionCache.clear();

    // Free all the ControlObjectScripts
    QList<ConfigKey> keys = m_controlCache.keys();
    QList<ConfigKey>::iterator it = keys.begin();
    QList<ConfigKey>::iterator end = keys.end();
    while (it != end) {
        ConfigKey key = *it;
        ControlObjectScript* coScript = m_controlCache.take(key);
        delete coScript;
        ++it;
    }
}

void ControllerEngine::initializeScriptEngine() {
    // Create the Script Engine
    m_pScriptEngine = new QJSEngine(this);

    // Make this ControllerEngine instance available to scripts as 'engine'.
    QJSValue engineGlobalObject = m_pScriptEngine->globalObject();
    ControllerEngineJSProxy* proxy = new ControllerEngineJSProxy(this);
    engineGlobalObject.setProperty("engine", m_pScriptEngine->newQObject(proxy));

    if (m_pController) {
        qDebug() << "Controller in script engine is:" << m_pController->getName();

        ControllerJSProxy* controllerProxy = m_pController->jsProxy();

        // Make the Controller instance available to scripts
        engineGlobalObject.setProperty("controller", m_pScriptEngine->newQObject(controllerProxy));

        // ...under the legacy name as well
        engineGlobalObject.setProperty("midi", m_pScriptEngine->newQObject(controllerProxy));
    }

    m_byteArrayToScriptValueJSFunction = evaluateCodeString("(function(arg1) { return new Uint8Array(arg1) })");
}

/* -------- ------------------------------------------------------
   Purpose: Load all script files given in the supplied list
   Input:   List of script paths and file names to load
   Output:  Returns true if no errors occurred.
   -------- ------------------------------------------------------ */
bool ControllerEngine::loadScriptFiles(const QList<QString>& scriptPaths,
                                       const QList<ControllerPreset::ScriptFileInfo>& scripts) {
    m_lastScriptPaths = scriptPaths;

    // scriptPaths holds the paths to search in when we're looking for scripts
    bool scriptsEvaluatedCorrectly = true;
    for (const ControllerPreset::ScriptFileInfo& script : scripts) {
        if (!evaluateScriptFile(script.name, scriptPaths)) {
            scriptsEvaluatedCorrectly = false;
        }
    }

    connect(&m_scriptWatcher, &QFileSystemWatcher::fileChanged,
            this, &ControllerEngine::scriptHasChanged);

    emit(initialized());

    return scriptsEvaluatedCorrectly;
}

// Slot to run when a script file has changed
void ControllerEngine::scriptHasChanged(const QString& scriptFilename) {
    Q_UNUSED(scriptFilename);
    disconnect(&m_scriptWatcher, &QFileSystemWatcher::fileChanged,
               this, &ControllerEngine::scriptHasChanged);
    reloadScripts();
}

void ControllerEngine::reloadScripts() {
    qDebug() << "ControllerEngine: Reloading Scripts";
    ControllerPresetPointer pPreset = m_pController->getPreset();

    gracefulShutdown();

    // Delete the script engine, first clearing the pointer so that
    // other threads will not get the dead pointer after we delete it.
    if (m_pScriptEngine != nullptr) {
        QJSEngine *engine = m_pScriptEngine;
        m_pScriptEngine = nullptr;
        engine->deleteLater();
    }

    initializeScriptEngine();
    loadScriptFiles(m_lastScriptPaths, pPreset->scripts);

    qDebug() << "Re-initializing scripts";
    initializeScripts(pPreset->scripts);
}

/* -------- ------------------------------------------------------
   Purpose: Run the initialization function for each loaded script
                if it exists
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
void ControllerEngine::initializeScripts(const QList<ControllerPreset::ScriptFileInfo>& scripts) {

    m_scriptFunctionPrefixes.clear();
    for (const ControllerPreset::ScriptFileInfo& script : scripts) {
        // Skip empty prefixes.
        if (!script.functionPrefix.isEmpty()) {
            m_scriptFunctionPrefixes.append(script.functionPrefix);
        }
    }

    QJSValueList args;
    args << QJSValue(m_pController->getName());
    args << QJSValue(ControllerDebug::enabled());

    // Call the init method for all the prefixes.
    callFunctionOnObjects(m_scriptFunctionPrefixes, "init", args);

    emit(initialized());
}

bool ControllerEngine::executeFunction(QJSValue functionObject, QJSValueList args) {
    VERIFY_OR_DEBUG_ASSERT(!(m_pScriptEngine == nullptr)) {
        return false;
    }

    if (functionObject.isError()) {
        qDebug() << "ControllerEngine::executeFunction:"
                 << functionObject.toString();
        return false;
    }

    // If it's not a function, we're done.
    if (!functionObject.isCallable()) {
        qDebug() << "ControllerEngine::executeFunction:"
                 << functionObject.toVariant()
                 << "Not a function";
        return false;
    }

    // If it does happen to be a function, call it.
    QJSValue returnValue = functionObject.call(args);
    if (returnValue.isError()) {
        showScriptExceptionDialog(returnValue);
        return false;
    }
    return true;
}

bool ControllerEngine::executeFunction(QJSValue functionObject, const QByteArray data) {
    VERIFY_OR_DEBUG_ASSERT(!(m_pScriptEngine == nullptr)) {
        return false;
    }
    QJSValueList args;
    args << byteArrayToScriptValue(data);
    args << QJSValue(data.size());
    return executeFunction(functionObject, args);
}

QJSValue ControllerEngine::evaluateCodeString(const QString& program, const QString& fileName,
        int lineNumber) {
    VERIFY_OR_DEBUG_ASSERT(!(m_pScriptEngine == nullptr)) {
        return QJSValue::UndefinedValue;
    }
    QJSValue returnValue = m_pScriptEngine->evaluate(program, fileName, lineNumber);
    return returnValue;
}

void ControllerEngine::throwJSError(const QString& message) {
  #if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
    QString errorText = tr("Uncaught exception: %1").arg(message);
    qWarning() << "ControllerEngine:" << errorText;
    if (!m_bDisplayingExceptionDialog) {
        scriptErrorDialog(errorText);
    }
  #else
    m_pScriptEngine->throwError(message);
  #endif
}

void ControllerEngine::showScriptExceptionDialog(QJSValue evaluationResult) {
    VERIFY_OR_DEBUG_ASSERT(evaluationResult.isError()) {
        return;
    }

    QString errorMessage = evaluationResult.toString();
    QString line = evaluationResult.property("lineNumber").toString();
    QString backtrace = evaluationResult.property("stack").toString();
    QString filename = evaluationResult.property("fileName").toString();

    QString errorText = tr("Uncaught exception at line %1 in file %2: %3")
            .arg(line, (filename.isEmpty() ? "" : filename), errorMessage);

    if (filename.isEmpty())
        errorText = tr("Uncaught exception at line %1 in passed code: %2")
                .arg(line, errorMessage);

    QString detailedError = ControllerDebug::enabled() ?
            QString("%1\nBacktrace:\n%2")
            .arg(errorText, backtrace) : errorText;

    qWarning() << "ControllerEngine:" << detailedError;

    if (!m_bDisplayingExceptionDialog) {
        scriptErrorDialog(detailedError);
    }
}

/*  -------- ------------------------------------------------------
    Purpose: Common error dialog creation code for run-time exceptions
                Allows users to ignore the error or reload the mappings
    Input:   Detailed error string
    Output:  -
    -------- ------------------------------------------------------ */
void ControllerEngine::scriptErrorDialog(const QString& detailedError) {
    ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
    props->setType(DLG_WARNING);
    props->setTitle(tr("Controller script error"));
    props->setText(tr("The mapping for the %1 controller is not working properly.").arg(m_pController->getName()));
    props->setInfoText("<html>"+tr("The script code needs to be fixed.")+
        "<p>"+tr("For now, you can: Ignore this error for this session but you may experience erratic behavior.")+
        "<br>"+tr("Try to recover by resetting your controller.")+"</p>"+"</html>");
    props->setDetails(detailedError);
    props->setKey(detailedError);   // To prevent multiple windows for the same error

    // Allow user to suppress further notifications about this particular error
    props->addButton(QMessageBox::Ignore);
    props->addButton(QMessageBox::Retry);
    props->setDefaultButton(QMessageBox::Ignore);
    props->setEscapeButton(QMessageBox::Ignore);
    props->setModal(false);

    if (ErrorDialogHandler::instance()->requestErrorDialog(props)) {
        m_bDisplayingExceptionDialog = true;
        // Enable custom handling of the dialog buttons
        connect(ErrorDialogHandler::instance(), &ErrorDialogHandler::stdButtonClicked,
                this, &ControllerEngine::errorDialogButton);
    }
}

/* -------- ------------------------------------------------------
    Purpose: Slot to handle custom button clicks in error dialogs
    Input:   Key of dialog, StandardButton that was clicked
    Output:  -
    -------- ------------------------------------------------------ */
void ControllerEngine::errorDialogButton(const QString& key, QMessageBox::StandardButton button) {
    Q_UNUSED(key);

    m_bDisplayingExceptionDialog = false;
    // Something was clicked, so disable this signal now
    disconnect(ErrorDialogHandler::instance(), &ErrorDialogHandler::stdButtonClicked,
               this, &ControllerEngine::errorDialogButton);

    if (button == QMessageBox::Retry) {
        reloadScripts();
    }
}

ControlObjectScript* ControllerEngine::getControlObjectScript(const QString& group, const QString& name) {
    ConfigKey key = ConfigKey(group, name);
    ControlObjectScript* coScript = m_controlCache.value(key, nullptr);
    if (coScript == nullptr) {
        // create COT
        coScript = new ControlObjectScript(key, this);
        if (coScript->valid()) {
            m_controlCache.insert(key, coScript);
        } else {
            delete coScript;
            coScript = nullptr;
        }
    }
    return coScript;
}

/* -------- ------------------------------------------------------
   Purpose: Returns the current value of a Mixxx control (for scripts)
   Input:   Control group (e.g. [Channel1]), Key name (e.g. [filterHigh])
   Output:  The value
   -------- ------------------------------------------------------ */
double ControllerEngine::getValue(QString group, QString name) {
    ControlObjectScript* coScript = getControlObjectScript(group, name);
    if (coScript == nullptr) {
        qWarning() << "ControllerEngine: Unknown control" << group << name << ", returning 0.0";
        return 0.0;
    }
    return coScript->get();
}

/* -------- ------------------------------------------------------
   Purpose: Sets new value of a Mixxx control (for scripts)
   Input:   Control group, Key name, new value
   Output:  -
   -------- ------------------------------------------------------ */
void ControllerEngine::setValue(QString group, QString name, double newValue) {
    if (isnan(newValue)) {
        qWarning() << "ControllerEngine: script setting [" << group << "," << name
                 << "] to NotANumber, ignoring.";
        return;
    }

    ControlObjectScript* coScript = getControlObjectScript(group, name);

    if (coScript != nullptr) {
        ControlObject* pControl = ControlObject::getControl(coScript->getKey());
        if (pControl && !m_st.ignore(pControl, coScript->getParameterForValue(newValue))) {
            coScript->slotSet(newValue);
        }
    }
}


/* -------- ------------------------------------------------------
   Purpose: Returns the normalized value of a Mixxx control (for scripts)
   Input:   Control group (e.g. [Channel1]), Key name (e.g. [filterHigh])
   Output:  The value
   -------- ------------------------------------------------------ */
double ControllerEngine::getParameter(QString group, QString name) {
    ControlObjectScript* coScript = getControlObjectScript(group, name);
    if (coScript == nullptr) {
        qWarning() << "ControllerEngine: Unknown control" << group << name << ", returning 0.0";
        return 0.0;
    }
    return coScript->getParameter();
}

/* -------- ------------------------------------------------------
   Purpose: Sets new normalized parameter of a Mixxx control (for scripts)
   Input:   Control group, Key name, new value
   Output:  -
   -------- ------------------------------------------------------ */
void ControllerEngine::setParameter(QString group, QString name, double newParameter) {
    if (isnan(newParameter)) {
        qWarning() << "ControllerEngine: script setting [" << group << "," << name
                 << "] to NotANumber, ignoring.";
        return;
    }

    ControlObjectScript* coScript = getControlObjectScript(group, name);

    if (coScript != nullptr) {
        ControlObject* pControl = ControlObject::getControl(coScript->getKey());
        if (pControl && !m_st.ignore(pControl, newParameter)) {
          coScript->setParameter(newParameter);
        }
    }
}

/* -------- ------------------------------------------------------
   Purpose: normalize a value of a Mixxx control (for scripts)
   Input:   Control group, Key name, new value
   Output:  -
   -------- ------------------------------------------------------ */
double ControllerEngine::getParameterForValue(QString group, QString name, double value) {
    if (isnan(value)) {
        qWarning() << "ControllerEngine: script setting [" << group << "," << name
                 << "] to NotANumber, ignoring.";
        return 0.0;
    }

    ControlObjectScript* coScript = getControlObjectScript(group, name);

    if (coScript == nullptr) {
        qWarning() << "ControllerEngine: Unknown control" << group << name << ", returning 0.0";
        return 0.0;
    }

    return coScript->getParameterForValue(value);
}

/* -------- ------------------------------------------------------
   Purpose: Resets the value of a Mixxx control (for scripts)
   Input:   Control group, Key name, new value
   Output:  -
   -------- ------------------------------------------------------ */
void ControllerEngine::reset(QString group, QString name) {
    ControlObjectScript* coScript = getControlObjectScript(group, name);
    if (coScript != nullptr) {
        coScript->reset();
    }
}

/* -------- ------------------------------------------------------
   Purpose: default value of a Mixxx control (for scripts)
   Input:   Control group, Key name, new value
   Output:  -
   -------- ------------------------------------------------------ */
double ControllerEngine::getDefaultValue(QString group, QString name) {
    ControlObjectScript* coScript = getControlObjectScript(group, name);

    if (coScript == nullptr) {
        qWarning() << "ControllerEngine: Unknown control" << group << name << ", returning 0.0";
        return 0.0;
    }

    return coScript->getDefault();
}

/* -------- ------------------------------------------------------
   Purpose: default parameter of a Mixxx control (for scripts)
   Input:   Control group, Key name, new value
   Output:  -
   -------- ------------------------------------------------------ */
double ControllerEngine::getDefaultParameter(QString group, QString name) {
    ControlObjectScript* coScript = getControlObjectScript(group, name);

    if (coScript == nullptr) {
        qWarning() << "ControllerEngine: Unknown control" << group << name << ", returning 0.0";
        return 0.0;
    }

    return coScript->getParameterForValue(coScript->getDefault());
}

/* -------- ------------------------------------------------------
   Purpose: qDebugs script output so it ends up in mixxx.log
   Input:   String to log
   Output:  -
   -------- ------------------------------------------------------ */
void ControllerEngine::log(QString message) {
    controllerDebug(message);
}

// Purpose: Connect a ControlObject's valueChanged() signal to a script callback function
// Input:   Control group (e.g. '[Channel1]'), Key name (e.g. 'pfl'), script callback
// Output:  a ScriptConnectionInvokableWrapper turned into a QJSValue.
//          The script should store this object to call its
//          'disconnect' and 'trigger' methods as needed.
//          If unsuccessful, returns undefined.
QJSValue ControllerEngine::makeConnection(QString group, QString name,
                                              const QJSValue callback) {
    VERIFY_OR_DEBUG_ASSERT(m_pScriptEngine != nullptr) {
        return QJSValue();
    }

    ControlObjectScript* coScript = getControlObjectScript(group, name);
    if (coScript == nullptr) {
        // The test setups do not run all of Mixxx, so ControlObjects not
        // existing during tests is okay.
        if (!m_bTesting) {
            throwJSError("ControllerEngine: script tried to connect to ControlObject (" +
                    group + ", " + name +
                    ") which is non-existent.");
        }
        return QJSValue();
    }

    if (!callback.isCallable()) {
        throwJSError("Tried to connect (" + group + ", " + name + ")"
                + " to an invalid callback.");
        return QJSValue();
    }

    ScriptConnection connection;
    connection.key = ConfigKey(group, name);
    connection.controllerEngine = this;
    connection.callback = callback;
    connection.id = QUuid::createUuid();

    if (coScript->addScriptConnection(connection)) {
        return m_pScriptEngine->newQObject(new ScriptConnectionInvokableWrapper(connection));
    }

    return QJSValue();
}

/* -------- ------------------------------------------------------
   Purpose: Execute a ScriptConnection's callback
   Input:   the value of the connected ControlObject to pass to the callback
   -------- ------------------------------------------------------ */
void ScriptConnection::executeCallback(double value) const {
    QJSValueList args;
    args << QJSValue(value);
    args << QJSValue(key.group);
    args << QJSValue(key.item);
    QJSValue func = callback; // copy function because QJSValue::call is not const
    QJSValue result = func.call(args);
    if (result.isError()) {
        if (controllerEngine != nullptr) {
            controllerEngine->showScriptExceptionDialog(result);
        }
        qWarning() << "ControllerEngine: Invocation of connection " << id.toString()
                   << "connected to (" + key.group + ", " + key.item + ") failed:"
                   << result.toString();
    }
}

/* -------- ------------------------------------------------------
   Purpose: (Dis)connects a ScriptConnection
   Input:   the ScriptConnection to disconnect
   -------- ------------------------------------------------------ */
void ControllerEngine::removeScriptConnection(const ScriptConnection connection) {
    ControlObjectScript* coScript = getControlObjectScript(connection.key.group,
                                                           connection.key.item);

    if (m_pScriptEngine == nullptr || coScript == nullptr) {
        return;
    }

    coScript->removeScriptConnection(connection);
}

void ScriptConnectionInvokableWrapper::disconnect() {
    m_scriptConnection.controllerEngine->removeScriptConnection(m_scriptConnection);
}

/* -------- ------------------------------------------------------
   Purpose: Triggers the callback function of a ScriptConnection
   Input:   the ScriptConnection to trigger
   -------- ------------------------------------------------------ */
void ControllerEngine::triggerScriptConnection(const ScriptConnection connection) {
    VERIFY_OR_DEBUG_ASSERT(!(m_pScriptEngine == nullptr)) {
        return;
    }

    ControlObjectScript* coScript = getControlObjectScript(connection.key.group,
                                                           connection.key.item);
    if (coScript == nullptr) {
        return;
    }

    connection.executeCallback(coScript->get());
}

void ScriptConnectionInvokableWrapper::trigger() {
    m_scriptConnection.controllerEngine->triggerScriptConnection(m_scriptConnection);
}

// This function is a legacy version of makeConnection with several alternate
// ways of invoking it. The callback function can be passed either as a string of
// JavaScript code that evaluates to a function or an actual JavaScript function.
// If "true" is passed as a 4th parameter, all connections to the ControlObject
// are removed. If a ScriptConnectionInvokableWrapper is passed instead of a callback,
// it is disconnected.
// WARNING: These behaviors are quirky and confusing, so if you change this function,
// be sure to run the ControllerEngineTest suite to make sure you do not break old scripts.
QJSValue ControllerEngine::connectControl(
        QString group, QString name, const QJSValue passedCallback, bool disconnect) {
    // The passedCallback may or may not actually be a function, so when
    // the actual callback function is found, store it in this variable.
    QJSValue actualCallbackFunction;

    if (passedCallback.isCallable()) {
        if (!disconnect) {
            // skip all the checks below and just make the connection
            return makeConnection(group, name, passedCallback);
        }
        actualCallbackFunction = passedCallback;
    }

    ControlObjectScript* coScript = getControlObjectScript(group, name);
    // This check is redundant with makeConnection, but the
    // ControlObjectScript is also needed here to check for duplicate connections.
    if (coScript == nullptr) {
        // The test setups do not run all of Mixxx, so ControlObjects not
        // existing during tests is okay.
        if (!m_bTesting) {
            if (disconnect) {
                throwJSError("ControllerEngine: script tried to disconnect from ControlObject (" +
                              group + ", " + name + ") which is non-existent.");
            } else {
                throwJSError("ControllerEngine: script tried to connect to ControlObject (" +
                              group + ", " + name + ") which is non-existent.");
            }
        }
        // This is inconsistent with other failures, which return false.
        // QJSValue() with no arguments is undefined in JavaScript.
        return QJSValue();
    }

    if (passedCallback.isString()) {
        // This check is redundant with makeConnection, but it must be done here
        // before evaluating the code string.
        VERIFY_OR_DEBUG_ASSERT(m_pScriptEngine != nullptr) {
            return QJSValue(false);
        }

        actualCallbackFunction = evaluateCodeString(passedCallback.toString());

        if (!actualCallbackFunction.isCallable()) {
            QString sErrorMessage("Invalid connection callback provided to engine.connectControl.");
            if (actualCallbackFunction.isError()) {
                sErrorMessage.append("\n" + actualCallbackFunction.toString());
            }
            throwJSError(sErrorMessage);
            return QJSValue(false);
        }

        if (coScript->countConnections() > 0 && !disconnect) {
            // This is inconsistent with the behavior when passing the callback as
            // a function, but keep the old behavior to make sure old scripts do
            // not break.
            ScriptConnection connection = coScript->firstConnection();

            qWarning() << "Tried to make duplicate connection between (" +
                          group + ", " + name + ") and " + passedCallback.toString() +
                          " but this is not allowed when passing a callback as a string. " +
                          "If you actually want to create duplicate connections, " +
                          "use engine.makeConnection. Returning reference to connection " +
                          connection.id.toString();

            return m_pScriptEngine->newQObject(new ScriptConnectionInvokableWrapper(connection));
        }
    } else if (passedCallback.isQObject()) {
        // Assume a ScriptConnection and assume that the script author
        // wants to disconnect it, regardless of the disconnect parameter
        // and regardless of whether it is connected to the same ControlObject
        // specified by the first two parameters to this function.
        QObject *qobject = passedCallback.toQObject();
        const QMetaObject *qmeta = qobject->metaObject();

        qWarning() << "QObject passed to engine.connectControl. Assuming it is"
                  << "a connection object to disconnect and returning false.";
        if (!strcmp(qmeta->className(),
                "ScriptConnectionInvokableWrapper")) {
            ScriptConnectionInvokableWrapper* proxy =
                    (ScriptConnectionInvokableWrapper*)qobject;
            proxy->disconnect();
        }
        return QJSValue(false);
    }

    // Support removing connections by passing "true" as the last parameter
    // to this function, regardless of whether the callback is provided
    // as a function or a string.
    if (disconnect) {
        // There is no way to determine which
        // ScriptConnection to disconnect unless the script calls
        // ScriptConnectionInvokableWrapper::disconnect(), so
        // disconnect all ScriptConnections connected to the
        // callback function, even though there may be multiple connections.
        coScript->disconnectAllConnectionsToFunction(actualCallbackFunction);
        return QJSValue(true);
    }

    // If execution gets this far without returning, make
    // a new connection to actualCallbackFunction.
    return makeConnection(group, name, actualCallbackFunction);
}

/* -------- ------------------------------------------------------
   DEPRECATED: Use ScriptConnectionInvokableWrapper::trigger instead
   Purpose: Emits valueChanged() so all ScriptConnections held by a
            ControlObjectScript have their callback executed
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
void ControllerEngine::trigger(QString group, QString name) {
    ControlObjectScript* coScript = getControlObjectScript(group, name);
    if (coScript != nullptr) {
        coScript->emitValueChanged();
    }
}

/* -------- ------------------------------------------------------
   Purpose: Evaluate a script file
   Input:   Script filename
   Output:  false if the script file has errors or doesn't exist
   -------- ------------------------------------------------------ */
bool ControllerEngine::evaluateScriptFile(const QString& scriptName, QList<QString> scriptPaths) {
    VERIFY_OR_DEBUG_ASSERT(!(m_pScriptEngine == nullptr)) {
        return false;
    }

    QString filename = "";
    QFile input;

    if (scriptPaths.length() == 0) {
        // If we aren't given any paths to search, assume that scriptName
        // contains the full file name
        filename = scriptName;
        input.setFileName(filename);
    } else {
        for (const QString& scriptPath : scriptPaths) {
            QDir scriptPathDir(scriptPath);
            filename = scriptPathDir.absoluteFilePath(scriptName);
            input.setFileName(filename);
            if (input.exists())  {
                qDebug() << "ControllerEngine: Watching JS File:" << filename;
                m_scriptWatcher.addPath(filename);
                break;
            }
        }
    }

    qDebug() << "ControllerEngine: Loading" << filename;

    // Read in the script file
    if (!input.open(QIODevice::ReadOnly)) {
        qWarning() << QString("ControllerEngine: Problem opening the script file: %1, error # %2, %3")
                .arg(filename, QString::number(input.error()), input.errorString());
        // Set up error dialog
        ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
        props->setType(DLG_WARNING);
        props->setTitle("Controller script file problem");
        props->setText(QString("There was a problem opening the controller script file %1.").arg(filename));
        props->setInfoText(input.errorString());

        // Ask above layer to display the dialog & handle user response
        ErrorDialogHandler::instance()->requestErrorDialog(props);
        return false;
    }

    QString scriptCode = "";
    scriptCode.append(input.readAll());
    scriptCode.append('\n');
    input.close();

    // Evaluate the code
    QJSValue scriptFunction = evaluateCodeString(scriptCode, filename);
    if (scriptFunction.isError()) {
        showScriptExceptionDialog(scriptFunction);
        return false;
    }

    return true;
}

/* -------- ------------------------------------------------------
   Purpose: Creates & starts a timer that runs some script code
                on timeout
   Input:   Number of milliseconds, script function to call,
                whether it should fire just once
   Output:  The timer's ID, 0 if starting it failed
   -------- ------------------------------------------------------ */
int ControllerEngine::beginTimer(int interval, QJSValue timerCallback,
                                 bool oneShot) {
    if (timerCallback.isString()) {
        timerCallback = evaluateCodeString(timerCallback.toString());
    }

    if (!timerCallback.isCallable()) {
        QString sErrorMessage("Invalid timer callback provided to engine.beginTimer. Valid callbacks are strings and functions.");
        if (timerCallback.isError()) {
            sErrorMessage.append("\n" + timerCallback.toString());
        }
        throwJSError(sErrorMessage);
        return 0;
    }

    if (interval < 20) {
        qWarning() << "Timer request for" << interval
                   << "ms is too short. Setting to the minimum of 20ms.";
        interval = 20;
    }

    // This makes use of every QObject's internal timer mechanism. Nice, clean,
    // and simple. See http://doc.trolltech.com/4.6/qobject.html#startTimer for
    // details
    int timerId = startTimer(interval);
    TimerInfo info;
    info.callback = timerCallback;
    info.oneShot = oneShot;
    m_timers[timerId] = info;
    if (timerId == 0) {
        qWarning() << "Script timer could not be created";
    } else if (oneShot) {
        controllerDebug("Starting one-shot timer:" << timerId);
    } else {
        controllerDebug("Starting timer:" << timerId);
    }
    return timerId;
}

/* -------- ------------------------------------------------------
   Purpose: Stops & removes a timer
   Input:   ID of timer to stop
   Output:  -
   -------- ------------------------------------------------------ */
void ControllerEngine::stopTimer(int timerId) {
    if (!m_timers.contains(timerId)) {
        qWarning() << "Killing timer" << timerId << ": That timer does not exist!";
        return;
    }
    controllerDebug("Killing timer:" << timerId);
    killTimer(timerId);
    m_timers.remove(timerId);
}

void ControllerEngine::stopAllTimers() {
    QMutableHashIterator<int, TimerInfo> i(m_timers);
    while (i.hasNext()) {
        i.next();
        stopTimer(i.key());
    }
}

void ControllerEngine::timerEvent(QTimerEvent *event) {
    int timerId = event->timerId();

    // See if this is a scratching timer
    if (m_scratchTimers.contains(timerId)) {
        scratchProcess(timerId);
        return;
    }

    auto it = m_timers.constFind(timerId);
    if (it == m_timers.constEnd()) {
        qWarning() << "Timer" << timerId << "fired but there's no function mapped to it!";
        return;
    }

    // NOTE(rryan): Do not assign by reference -- make a copy. I have no idea
    // why but this causes segfaults in ~QScriptValue while scratching if we
    // don't copy here -- even though internalExecute passes the QScriptValues
    // by value. *boggle*
    const TimerInfo timerTarget = it.value();
    if (timerTarget.oneShot) {
        stopTimer(timerId);
    }

    executeFunction(timerTarget.callback, QJSValueList());
}

double ControllerEngine::getDeckRate(const QString& group) {
    double rate = 0.0;
    ControlObjectScript* pRate = getControlObjectScript(group, "rate");
    if (pRate != nullptr) {
        rate = pRate->get();
    }
    ControlObjectScript* pRateDir = getControlObjectScript(group, "rate_dir");
    if (pRateDir != nullptr) {
        rate *= pRateDir->get();
    }
    ControlObjectScript* pRateRange = getControlObjectScript(group, "rateRange");
    if (pRateRange != nullptr) {
        rate *= pRateRange->get();
    }

    // Add 1 since the deck is playing
    rate += 1.0;

    // See if we're in reverse play
    ControlObjectScript* pReverse = getControlObjectScript(group, "reverse");
    if (pReverse != nullptr && pReverse->get() == 1) {
        rate = -rate;
    }
    return rate;
}

bool ControllerEngine::isDeckPlaying(const QString& group) {
    ControlObjectScript* pPlay = getControlObjectScript(group, "play");

    if (pPlay == nullptr) {
      QString error = QString("Could not getControlObjectScript()");
      scriptErrorDialog(error);
      return false;
    }

    return pPlay->get() > 0.0;
}

/* -------- ------------------------------------------------------
    Purpose: Enables scratching for relative controls
    Input:   Virtual deck to scratch,
             Number of intervals per revolution of the controller wheel,
             RPM for the track at normal speed (usually 33+1/3),
             (optional) alpha value for the filter,
             (optional) beta value for the filter
    Output:  -
    -------- ------------------------------------------------------ */
void ControllerEngine::scratchEnable(int deck, int intervalsPerRev, double rpm,
                                     double alpha, double beta, bool ramp) {
    // If we're already scratching this deck, override that with this request
    if (m_dx[deck]) {
        //qDebug() << "Already scratching deck" << deck << ". Overriding.";
        int timerId = m_scratchTimers.key(deck);
        killTimer(timerId);
        m_scratchTimers.remove(timerId);
    }

    // Controller resolution in intervals per second at normal speed.
    // (rev/min * ints/rev * mins/sec)
    double intervalsPerSecond = (rpm * intervalsPerRev) / 60.0;

    if (intervalsPerSecond == 0.0) {
        qWarning() << "Invalid rpm or intervalsPerRev supplied to scratchEnable. Ignoring request.";
        return;
    }

    m_dx[deck] = 1.0 / intervalsPerSecond;
    m_intervalAccumulator[deck] = 0.0;
    m_ramp[deck] = false;
    m_rampFactor[deck] = 0.001;
    m_brakeActive[deck] = false;

    // PlayerManager::groupForDeck is 0-indexed.
    QString group = PlayerManager::groupForDeck(deck - 1);

    // Ramp velocity, default to stopped.
    double initVelocity = 0.0;

    ControlObjectScript* pScratch2Enable =
            getControlObjectScript(group, "scratch2_enable");

    // If ramping is desired, figure out the deck's current speed
    if (ramp) {
        // See if the deck is already being scratched
        if (pScratch2Enable != nullptr && pScratch2Enable->get() == 1) {
            // If so, set the filter's initial velocity to the scratch speed
            ControlObjectScript* pScratch2 =
                    getControlObjectScript(group, "scratch2");
            if (pScratch2 != nullptr) {
                initVelocity = pScratch2->get();
            }
        } else if (isDeckPlaying(group)) {
            // If the deck is playing, set the filter's initial velocity to the
            // playback speed
            initVelocity = getDeckRate(group);
        }
    }

    // Initialize scratch filter
    if (alpha && beta) {
        m_scratchFilters[deck]->init(kAlphaBetaDt, initVelocity, alpha, beta);
    } else {
        // Use filter's defaults if not specified
        m_scratchFilters[deck]->init(kAlphaBetaDt, initVelocity);
    }

    // 1ms is shortest possible, OS dependent
    int timerId = startTimer(kScratchTimerMs);

    // Associate this virtual deck with this timer for later processing
    m_scratchTimers[timerId] = deck;

    // Set scratch2_enable
    if (pScratch2Enable != nullptr) {
        pScratch2Enable->slotSet(1);
    }
}

/* -------- ------------------------------------------------------
    Purpose: Accumulates "ticks" of the controller wheel
    Input:   Virtual deck to scratch, interval value (usually +1 or -1)
    Output:  -
    -------- ------------------------------------------------------ */
void ControllerEngine::scratchTick(int deck, int interval) {
    m_lastMovement[deck] = mixxx::Time::elapsed();
    m_intervalAccumulator[deck] += interval;
}

/* -------- ------------------------------------------------------
    Purpose: Applies the accumulated movement to the track speed
    Input:   ID of timer for this deck
    Output:  -
    -------- ------------------------------------------------------ */
void ControllerEngine::scratchProcess(int timerId) {
    int deck = m_scratchTimers[timerId];
    // PlayerManager::groupForDeck is 0-indexed.
    QString group = PlayerManager::groupForDeck(deck - 1);
    AlphaBetaFilter* filter = m_scratchFilters[deck];
    if (!filter) {
        qWarning() << "Scratch filter pointer is null on deck" << deck;
        return;
    }

    const double oldRate = filter->predictedVelocity();

    // Give the filter a data point:

    // If we're ramping to end scratching and the wheel hasn't been turned very
    // recently (spinback after lift-off,) feed fixed data
    if (m_ramp[deck] && !m_softStartActive[deck] &&
        ((mixxx::Time::elapsed() - m_lastMovement[deck]) >= mixxx::Duration::fromMillis(1))) {
        filter->observation(m_rampTo[deck] * m_rampFactor[deck]);
        // Once this code path is run, latch so it always runs until reset
        //m_lastMovement[deck] += mixxx::Duration::fromSeconds(1);
    } else if (m_softStartActive[deck]) {
        // pretend we have moved by (desired rate*default distance)
        filter->observation(m_rampTo[deck]*kAlphaBetaDt);
    } else {
        // This will (and should) be 0 if no net ticks have been accumulated
        // (i.e. the wheel is stopped)
        filter->observation(m_dx[deck] * m_intervalAccumulator[deck]);
    }

    const double newRate = filter->predictedVelocity();

    // Actually do the scratching
    ControlObjectScript* pScratch2 = getControlObjectScript(group, "scratch2");
    if (pScratch2 == nullptr) {
        return; // abort and maybe it'll work on the next pass
    }
    pScratch2->set(newRate);

    // Reset accumulator
    m_intervalAccumulator[deck] = 0;

    // End scratching if we're ramping and the current rate is really close to the rampTo value
    if ((m_ramp[deck] && fabs(m_rampTo[deck] - newRate) <= 0.00001) ||
        // or if we brake or softStart and have crossed over the desired value,
        ((m_brakeActive[deck] || m_softStartActive[deck]) && (
            (oldRate > m_rampTo[deck] && newRate < m_rampTo[deck]) ||
            (oldRate < m_rampTo[deck] && newRate > m_rampTo[deck]))) ||
        // or if the deck was stopped manually during brake or softStart
        ((m_brakeActive[deck] || m_softStartActive[deck]) && (!isDeckPlaying(group)))) {
        // Not ramping no mo'
        m_ramp[deck] = false;

        if (m_brakeActive[deck]) {
            // If in brake mode, set scratch2 rate to 0 and turn off the play button.
            pScratch2->slotSet(0.0);
            ControlObjectScript* pPlay = getControlObjectScript(group, "play");
            if (pPlay != nullptr) {
                pPlay->slotSet(0.0);
            }
        }

        // Clear scratch2_enable to end scratching.
        ControlObjectScript* pScratch2Enable =
                getControlObjectScript(group, "scratch2_enable");
        if (pScratch2Enable == nullptr) {
            return; // abort and maybe it'll work on the next pass
        }
        pScratch2Enable->slotSet(0);

        // Remove timer
        killTimer(timerId);
        m_scratchTimers.remove(timerId);

        m_dx[deck] = 0.0;
        m_brakeActive[deck] = false;
        m_softStartActive[deck] = false;
    }
}

/* -------- ------------------------------------------------------
    Purpose: Stops scratching the specified virtual deck
    Input:   Virtual deck to stop scratching
    Output:  -
    -------- ------------------------------------------------------ */
void ControllerEngine::scratchDisable(int deck, bool ramp) {
    // PlayerManager::groupForDeck is 0-indexed.
    QString group = PlayerManager::groupForDeck(deck - 1);

    m_rampTo[deck] = 0.0;

    // If no ramping is desired, disable scratching immediately
    if (!ramp) {
        // Clear scratch2_enable
        ControlObjectScript* pScratch2Enable = getControlObjectScript(group, "scratch2_enable");
        if (pScratch2Enable != nullptr) {
            pScratch2Enable->slotSet(0);
        }
        // Can't return here because we need scratchProcess to stop the timer.
        // So it's still actually ramping, we just won't hear or see it.
    } else if (isDeckPlaying(group)) {
        // If so, set the target velocity to the playback speed
        m_rampTo[deck] = getDeckRate(group);
    }

    m_lastMovement[deck] = mixxx::Time::elapsed();
    m_ramp[deck] = true;    // Activate the ramping in scratchProcess()
}

/* -------- ------------------------------------------------------
    Purpose: Tells if the specified deck is currently scratching
             (Scripts need this to implement spinback-after-lift-off)
    Input:   Virtual deck to inquire about
    Output:  True if so
    -------- ------------------------------------------------------ */
bool ControllerEngine::isScratching(int deck) {
    // PlayerManager::groupForDeck is 0-indexed.
    QString group = PlayerManager::groupForDeck(deck - 1);
    return getValue(group, "scratch2_enable") > 0;
}

/*  -------- ------------------------------------------------------
    Purpose: [En/dis]ables soft-takeover status for a particular ControlObject
    Input:   ControlObject group and key values,
                whether to set the soft-takeover status or not
    Output:  -
    -------- ------------------------------------------------------ */
void ControllerEngine::softTakeover(QString group, QString name, bool set) {
    ControlObject* pControl = ControlObject::getControl(ConfigKey(group, name));
    if (!pControl) {
        return;
    }
    if (set) {
        m_st.enable(pControl);
    } else {
        m_st.disable(pControl);
    }
}

/*  -------- ------------------------------------------------------
     Purpose: Ignores the next value for the given ControlObject
                This should be called before or after an absolute physical
                control (slider or knob with hard limits) is changed to operate
                on a different ControlObject, allowing it to sync up to the
                soft-takeover state without an abrupt jump.
     Input:   ControlObject group and key values
     Output:  -
     -------- ------------------------------------------------------ */
void ControllerEngine::softTakeoverIgnoreNextValue(QString group, const QString name) {
    ControlObject* pControl = ControlObject::getControl(ConfigKey(group, name));
    if (!pControl) {
        return;
    }

    m_st.ignoreNext(pControl);
}

/*  -------- ------------------------------------------------------
    Purpose: [En/dis]ables spinback effect for the channel
    Input:   deck, activate/deactivate, factor (optional),
             rate (optional)
    Output:  -
    -------- ------------------------------------------------------ */
void ControllerEngine::spinback(int deck, bool activate, double factor, double rate) {
    // defaults for args set in header file
    brake(deck, activate, factor, rate);
}

/*  -------- ------------------------------------------------------
    Purpose: [En/dis]ables brake/spinback effect for the channel
    Input:   deck, activate/deactivate, factor (optional),
             rate (optional, necessary for spinback)
    Output:  -
    -------- ------------------------------------------------------ */
void ControllerEngine::brake(int deck, bool activate, double factor, double rate) {
    // PlayerManager::groupForDeck is 0-indexed.
    QString group = PlayerManager::groupForDeck(deck - 1);

    // kill timer when both enabling or disabling
    int timerId = m_scratchTimers.key(deck);
    killTimer(timerId);
    m_scratchTimers.remove(timerId);

    // enable/disable scratch2 mode
    ControlObjectScript* pScratch2Enable = getControlObjectScript(group, "scratch2_enable");
    if (pScratch2Enable != nullptr) {
        pScratch2Enable->slotSet(activate ? 1 : 0);
    }

    // used in scratchProcess for the different timer behavior we need
    m_brakeActive[deck] = activate;
    double initRate = rate;

    if (activate) {
        // store the new values for this spinback/brake effect
        if (initRate == 1.0) {// then rate is really 1.0 or was set to default
            // in /res/common-controller-scripts.js so check for real value,
            // taking pitch into account
            initRate = getDeckRate(group);
        }
        // stop ramping at a rate which doesn't produce any audible output anymore
        m_rampTo[deck] = 0.01;
        // if we are currently softStart()ing, stop it
        if (m_softStartActive[deck]) {
            m_softStartActive[deck] = false;
            AlphaBetaFilter* filter = m_scratchFilters[deck];
            if (filter != nullptr) {
                initRate = filter->predictedVelocity();
            }
        }

        // setup timer and set scratch2
        timerId = startTimer(kScratchTimerMs);
        m_scratchTimers[timerId] = deck;

        ControlObjectScript* pScratch2 = getControlObjectScript(group, "scratch2");
        if (pScratch2 != nullptr) {
            pScratch2->slotSet(initRate);
        }

        // setup the filter with default alpha and beta*factor
        double alphaBrake = 1.0/512;
        // avoid decimals for fine adjusting
        if (factor>1) {
            factor = ((factor-1)/10)+1;
        }
        double betaBrake = ((1.0/512)/1024)*factor; // default*factor
        AlphaBetaFilter* filter = m_scratchFilters[deck];
        if (filter != nullptr) {
            filter->init(kAlphaBetaDt, initRate, alphaBrake, betaBrake);
        }

        // activate the ramping in scratchProcess()
        m_ramp[deck] = true;
    }
}

/*  -------- ------------------------------------------------------
    Purpose: [En/dis]ables softStart effect for the channel
    Input:   deck, activate/deactivate, factor (optional)
    Output:  -
    -------- ------------------------------------------------------ */
void ControllerEngine::softStart(int deck, bool activate, double factor) {
    // PlayerManager::groupForDeck is 0-indexed.
    QString group = PlayerManager::groupForDeck(deck - 1);

    // kill timer when both enabling or disabling
    int timerId = m_scratchTimers.key(deck);
    killTimer(timerId);
    m_scratchTimers.remove(timerId);

    // enable/disable scratch2 mode
    ControlObjectScript* pScratch2Enable = getControlObjectScript(group, "scratch2_enable");
    if (pScratch2Enable != nullptr) {
        pScratch2Enable->slotSet(activate ? 1 : 0);
    }

    // used in scratchProcess for the different timer behavior we need
    m_softStartActive[deck] = activate;
    double initRate = 0.0;

    if (activate) {
        // acquire deck rate
        m_rampTo[deck] = getDeckRate(group);

        // if brake()ing, get current rate from filter
        if (m_brakeActive[deck]) {
            m_brakeActive[deck] = false;

            AlphaBetaFilter* filter = m_scratchFilters[deck];
            if (filter != nullptr) {
                initRate = filter->predictedVelocity();
            }
        }

        // setup timer, start playing and set scratch2
        timerId = startTimer(kScratchTimerMs);
        m_scratchTimers[timerId] = deck;

        ControlObjectScript* pPlay = getControlObjectScript(group, "play");
        if (pPlay != nullptr) {
            pPlay->slotSet(1.0);
        }

        ControlObjectScript* pScratch2 = getControlObjectScript(group, "scratch2");
        if (pScratch2 != nullptr) {
            pScratch2->slotSet(initRate);
        }

        // setup the filter like in brake(), with default alpha and beta*factor
        double alphaSoft = 1.0/512;
        // avoid decimals for fine adjusting
        if (factor>1) {
            factor = ((factor-1)/10)+1;
        }
        double betaSoft = ((1.0/512)/1024)*factor; // default: (1.0/512)/1024
        AlphaBetaFilter* filter = m_scratchFilters[deck];
        if (filter != nullptr) { // kAlphaBetaDt = 1/1000 seconds
            filter->init(kAlphaBetaDt, initRate, alphaSoft, betaSoft);
        }

        // activate the ramping in scratchProcess()
        m_ramp[deck] = true;
    }
}
