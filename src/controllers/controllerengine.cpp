/***************************************************************************
                          controllerengine.cpp  -  description
                          -------------------
    begin                : Sat Apr 30 2011
    copyright            : (C) 2011 by Sean M. Pappalardo
    email                : spappalardo@mixxx.org
 ***************************************************************************/

#include "controllers/colormapperjsproxy.h"
#include "controllers/controllerengine.h"
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

namespace {
constexpr int kDecks = 16;

// Use 1ms for the Alpha-Beta dt. We're assuming the OS actually gives us a 1ms
// timer.
constexpr int kScratchTimerMs = 1;
constexpr double kAlphaBetaDt = kScratchTimerMs / 1000.0;

inline ControlFlags onlyAssertOnControllerDebug() {
    if (ControllerDebug::enabled()) {
        return ControlFlag::None;
    }

    return ControlFlag::AllowMissingOrInvalid;
}
} // namespace

ControllerEngine::ControllerEngine(
        Controller* controller, UserSettingsPointer pConfig)
        : m_pEngine(nullptr),
          m_pController(controller),
          m_pConfig(pConfig),
          m_bPopups(true),
          m_pBaClass(nullptr) {
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

    uninitializeScriptEngine();
}

/* -------- ------------------------------------------------------
Purpose: Calls the same method on a list of JS Objects
Input:   -
Output:  -
-------- ------------------------------------------------------ */
void ControllerEngine::callFunctionOnObjects(QList<QString> scriptFunctionPrefixes,
                                             const QString& function, QScriptValueList args) {
    VERIFY_OR_DEBUG_ASSERT(m_pEngine) {
        return;
    }

    const QScriptValue global = m_pEngine->globalObject();

    for (const QString& prefixName : scriptFunctionPrefixes) {
        QScriptValue prefix = global.property(prefixName);
        if (!prefix.isValid() || !prefix.isObject()) {
            qWarning() << "ControllerEngine: No" << prefixName << "object in script";
            // Throw a debug assertion if controllerDebug is enabled
            DEBUG_ASSERT(!ControllerDebug::enabled());
            continue;
        }

        QScriptValue init = prefix.property(function);
        if (!init.isValid() || !init.isFunction()) {
            qWarning() << "ControllerEngine:" << prefixName << "has no" << function << " method";
            // Throw a debug assertion if controllerDebug is enabled
            DEBUG_ASSERT(!ControllerDebug::enabled());
            continue;
        }
        controllerDebug("ControllerEngine: Executing" << prefixName << "." << function);
        init.call(prefix, args);
    }
}

/* ------------------------------------------------------------------
Purpose: Turn a snippet of JS into a QScriptValue function.
         Wrapping it in an anonymous function allows any JS that
         evaluates to a function to be used in MIDI mapping XML files
         and ensures the function is executed with the correct
         'this' object.
Input:   QString snippet of JS that evaluates to a function,
         int number of arguments that the function takes
Output:  QScriptValue of JS snippet wrapped in an anonymous function
------------------------------------------------------------------- */
QScriptValue ControllerEngine::wrapFunctionCode(const QString& codeSnippet,
                                                int numberOfArgs) {
    // This function is called from outside the controller engine, so we can't
    // use VERIFY_OR_DEBUG_ASSERT here
    if (m_pEngine == nullptr) {
        return QScriptValue();
    }

    QScriptValue wrappedFunction;

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
        wrappedFunction = m_pEngine->evaluate(wrappedCode);
        checkException();
        m_scriptWrappedFunctionCache[codeSnippet] = wrappedFunction;
    }
    return wrappedFunction;
}

QScriptValue ControllerEngine::getThisObjectInFunctionCall() {
    VERIFY_OR_DEBUG_ASSERT(m_pEngine != nullptr) {
        return QScriptValue();
    }

    QScriptContext *ctxt = m_pEngine->currentContext();
    // Our current context is a function call. We want to grab the 'this'
    // from the caller's context, so we walk up the stack.
    if (ctxt) {
        ctxt = ctxt->parentContext();
    }
    return ctxt ? ctxt->thisObject() : QScriptValue();
}

/* -------- ------------------------------------------------------
Purpose: Shuts down scripts in an orderly fashion
            (stops timers then executes shutdown functions)
Input:   -
Output:  -
-------- ------------------------------------------------------ */
void ControllerEngine::gracefulShutdown() {
    if (m_pEngine == nullptr) {
        return;
    }

    qDebug() << "ControllerEngine shutting down...";

    // Stop all timers
    stopAllTimers();

    qDebug() << "Invoking shutdown() hook in scripts";
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

    qDebug() << "Clearing function wrapper cache";
    m_scriptWrappedFunctionCache.clear();

    // Free all the ControlObjectScripts
    {
        auto it = m_controlCache.begin();
        while (it != m_controlCache.end()) {
            qDebug()
                    << "Deleting ControlObjectScript"
                    << it.key().group
                    << it.key().item;
            delete it.value();
            // Advance iterator
            it = m_controlCache.erase(it);
        }
    }

    delete m_pBaClass;
    m_pBaClass = nullptr;
}

bool ControllerEngine::isReady() {
    bool ret = m_pEngine != nullptr;
    return ret;
}

void ControllerEngine::initializeScriptEngine() {
    // Clear any errors from previous script engine usages
    m_scriptErrors.clear();

    // Create the Script Engine
    m_pEngine = new QScriptEngine(this);

    // Make this ControllerEngine instance available to scripts as 'engine'.
    QScriptValue engineGlobalObject = m_pEngine->globalObject();
    engineGlobalObject.setProperty("engine", m_pEngine->newQObject(this));

    if (m_pController) {
        qDebug() << "Controller in script engine is:" << m_pController->getName();

        // Make the Controller instance available to scripts
        engineGlobalObject.setProperty("controller", m_pEngine->newQObject(m_pController));

        // ...under the legacy name as well
        engineGlobalObject.setProperty("midi", m_pEngine->newQObject(m_pController));
    }

    QScriptValue constructor = m_pEngine->newFunction(ColorMapperJSProxyConstructor);
    QScriptValue metaObject = m_pEngine->newQMetaObject(&ColorMapperJSProxy::staticMetaObject, constructor);
    engineGlobalObject.setProperty("ColorMapper", metaObject);

    m_pBaClass = new ByteArrayClass(m_pEngine);
    engineGlobalObject.setProperty("ByteArray", m_pBaClass->constructor());
}

void ControllerEngine::uninitializeScriptEngine() {
    // Delete the script engine, first clearing the pointer so that
    // other threads will not get the dead pointer after we delete it.
    if (m_pEngine != nullptr) {
        QScriptEngine* engine = m_pEngine;
        m_pEngine = nullptr;
        engine->deleteLater();
    }
}

/* -------- ------------------------------------------------------
   Purpose: Load all script files given in the supplied list
   Input:   List of script paths and file names to load
   Output:  Returns true if no errors occurred.
   -------- ------------------------------------------------------ */
bool ControllerEngine::loadScriptFiles(const QList<ControllerPreset::ScriptFileInfo>& scripts) {
    bool result = true;
    for (const auto& script : scripts) {
        if (!evaluate(script.file)) {
            result = false;
        }

        if (m_scriptErrors.contains(script.name)) {
            qWarning() << "Errors occurred while loading" << script.name;
        }
    }

    m_lastScriptFiles = scripts;

    connect(&m_scriptWatcher, SIGNAL(fileChanged(QString)),
            this, SLOT(scriptHasChanged(QString)));

    bool success = result && m_scriptErrors.isEmpty();
    if (!success) {
        gracefulShutdown();
        uninitializeScriptEngine();
    }

    return success;
}

// Slot to run when a script file has changed
void ControllerEngine::scriptHasChanged(const QString& scriptFilename) {
    Q_UNUSED(scriptFilename);
    qDebug() << "ControllerEngine: Reloading Scripts";
    ControllerPresetPointer pPreset = m_pController->getPreset();

    disconnect(&m_scriptWatcher, SIGNAL(fileChanged(QString)),
               this, SLOT(scriptHasChanged(QString)));

    gracefulShutdown();
    uninitializeScriptEngine();

    initializeScriptEngine();
    if (!loadScriptFiles(m_lastScriptFiles)) {
        return;
    }

    qDebug() << "Re-initializing scripts";
    initializeScripts(m_lastScriptFiles);
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

    QScriptValueList args;
    args << QScriptValue(m_pController->getName());
    args << QScriptValue(ControllerDebug::enabled());

    // Call the init method for all the prefixes.
    callFunctionOnObjects(m_scriptFunctionPrefixes, "init", args);

    // We failed to initialize the controller scripts, shutdown the script
    // engine to avoid error popups on every button press or slider move
    if (checkException(true)) {
        gracefulShutdown();
        uninitializeScriptEngine();
    }
}

/* -------- ------------------------------------------------------
   Purpose: Validate script syntax, then evaluate() it so the
            functions are registered & available for use.
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
bool ControllerEngine::evaluate(const QString& filepath) {
    return evaluate(QFileInfo(filepath));
}

bool ControllerEngine::syntaxIsValid(const QString& scriptCode, const QString& filename) {
    if (m_pEngine == nullptr) {
        return false;
    }

    QScriptSyntaxCheckResult result = m_pEngine->checkSyntax(scriptCode);

    // Note: Do not translate the error messages that go into the "details"
    // part of the error dialog. These serve as starting point for mapping
    // developers and might not always be fluent in the language of mapping
    // user.
    QString error;
    switch (result.state()) {
        case (QScriptSyntaxCheckResult::Valid): break;
        case (QScriptSyntaxCheckResult::Intermediate):
            error = QStringLiteral("Incomplete code");
            break;
        case (QScriptSyntaxCheckResult::Error):
            error = QStringLiteral("Syntax error");
            break;
    }

    // If we didn't encounter an error, exit early
    if (error.isEmpty()) {
        return true;
    }

    if (filename.isEmpty()) {
        error = QString("%1 at line %2, column %3")
                        .arg(error,
                                QString::number(result.errorLineNumber()),
                                QString::number(result.errorColumnNumber()));
    } else {
        error = QString("%1 at line %2, column %3 in file %4")
                        .arg(error,
                                QString::number(result.errorLineNumber()),
                                QString::number(result.errorColumnNumber()),
                                filename);
    }

    QString errorMessage = result.errorMessage();
    if (!errorMessage.isEmpty()) {
        error += QStringLiteral("\n\nError:  \n") + errorMessage;
    }

    if (filename.isEmpty()) {
        error += QStringLiteral("\n\nCode:\n") + scriptCode;
    }

    qWarning() << "ControllerEngine:" << error;
    scriptErrorDialog(error, error, true);
    return false;
}

/* -------- ------------------------------------------------------
Purpose: Evaluate & run script code
Input:   'this' object if applicable, Code string
Output:  false if an exception
-------- ------------------------------------------------------ */
bool ControllerEngine::internalExecute(
        QScriptValue thisObject, const QString& scriptCode) {
    // A special version of safeExecute since we're evaluating strings, not actual functions
    //  (execute() would print an error that it's not a function every time a timer fires.)
    if (m_pEngine == nullptr) {
        return false;
    }

    if (!syntaxIsValid(scriptCode)) {
        return false;
    }

    QScriptValue scriptFunction = m_pEngine->evaluate(scriptCode);

    if (checkException()) {
        qDebug() << "Exception evaluating:" << scriptCode;
        return false;
    }

    if (!scriptFunction.isFunction()) {
        // scriptCode was plain code called in evaluate above
        return false;
    }

    return internalExecute(thisObject, scriptFunction, QScriptValueList());
}

/* -------- ------------------------------------------------------
Purpose: Evaluate & run script code
Input:   'this' object if applicable, Code string
Output:  false if an exception
-------- ------------------------------------------------------ */
bool ControllerEngine::internalExecute(QScriptValue thisObject,
        QScriptValue functionObject,
        QScriptValueList args) {
    if (m_pEngine == nullptr) {
        qDebug() << "ControllerEngine::execute: No script engine exists!";
        return false;
    }

    if (functionObject.isError()) {
        qWarning() << "ControllerEngine::internalExecute:"
                   << functionObject.toString();
        // Throw a debug assertion if controllerDebug is enabled
        DEBUG_ASSERT(!ControllerDebug::enabled());
        return false;
    }

    // If it's not a function, we're done.
    if (!functionObject.isFunction()) {
        qWarning() << "ControllerEngine::internalExecute:"
                   << functionObject.toVariant() << "Not a function";
        // Throw a debug assertion if controllerDebug is enabled
        DEBUG_ASSERT(!ControllerDebug::enabled());
        return false;
    }

    // If it does happen to be a function, call it.
    QScriptValue rc = functionObject.call(thisObject, args);
    if (!rc.isValid()) {
        qWarning() << "QScriptValue is not a function or ...";
        // Throw a debug assertion if controllerDebug is enabled
        DEBUG_ASSERT(!ControllerDebug::enabled());
        return false;
    }

    return !checkException();
}

bool ControllerEngine::execute(QScriptValue functionObject,
        unsigned char channel,
        unsigned char control,
        unsigned char value,
        unsigned char status,
        const QString& group,
        mixxx::Duration timestamp) {
    Q_UNUSED(timestamp);
    if (m_pEngine == nullptr) {
        return false;
    }
    QScriptValueList args;
    args << QScriptValue(channel);
    args << QScriptValue(control);
    args << QScriptValue(value);
    args << QScriptValue(status);
    args << QScriptValue(group);
    return internalExecute(m_pEngine->globalObject(), functionObject, args);
}

bool ControllerEngine::execute(QScriptValue function,
        const QByteArray data,
        mixxx::Duration timestamp) {
    Q_UNUSED(timestamp);
    if (m_pEngine == nullptr) {
        return false;
    }
    QScriptValueList args;
    args << m_pBaClass->newInstance(data);
    args << QScriptValue(data.size());
    return internalExecute(m_pEngine->globalObject(), function, args);
}

/* -------- ------------------------------------------------------
   Purpose: Check to see if a script threw an exception
   Input:   QScriptValue returned from call(scriptFunctionName)
   Output:  true if there was an exception
   -------- ------------------------------------------------------ */
bool ControllerEngine::checkException(bool bFatal) {
    if (m_pEngine == nullptr) {
        return false;
    }

    if (m_pEngine->hasUncaughtException()) {
        QScriptValue exception = m_pEngine->uncaughtException();
        QString errorMessage = exception.toString();
        QString line =
                QString::number(m_pEngine->uncaughtExceptionLineNumber());
        QString filename = exception.property("fileName").toString();

        // Note: Do not translate the error messages that go into the "details"
        // part of the error dialog. These serve as starting point for mapping
        // developers and might not always be fluent in the language of mapping
        // user.
        QStringList error;
        error << (filename.isEmpty() ? "" : filename) << errorMessage << line;
        m_scriptErrors.insert(
                (filename.isEmpty() ? "passed code" : filename), error);

        QString errorText;
        if (filename.isEmpty()) {
            errorText = QString("Uncaught exception at line %1 in passed code.").arg(line);
        } else {
            errorText = QString("Uncaught exception at line %1 in file %2.").arg(line, filename);
        }

        errorText += QStringLiteral("\n\nException:\n  ") + errorMessage;

        // Do not include backtrace in dialog key because it might contain midi
        // slider values that will differ most of the time. This would break
        // the "Ignore" feature of the error dialog.
        QString key = errorText;

        // Add backtrace to the error details
        errorText += QStringLiteral("\n\nBacktrace:\n  ") +
                m_pEngine->uncaughtExceptionBacktrace().join("\n  ");

        scriptErrorDialog(errorText, key, bFatal);
        m_pEngine->clearExceptions();
        return true;
    }
    return false;
}

/*  -------- ------------------------------------------------------
    Purpose: Common error dialog creation code for run-time exceptions
                Allows users to ignore the error or reload the mappings
    Input:   Detailed error string
    Output:  -
    -------- ------------------------------------------------------ */
void ControllerEngine::scriptErrorDialog(
        const QString& detailedError, const QString& key, bool bFatalError) {
    qWarning() << "ControllerEngine:" << detailedError;

    if (!m_bPopups) {
        return;
    }

    ErrorDialogProperties* props =
            ErrorDialogHandler::instance()->newDialogProperties();

    QString additionalErrorText;
    if (bFatalError) {
        additionalErrorText =
                tr("The functionality provided by this controller mapping will "
                   "be disabled until the issue has been resolved.");
    } else {
        additionalErrorText =
                tr("You can ignore this error for this session but "
                   "you may experience erratic behavior.") +
                QString("<br>") +
                tr("Try to recover by resetting your controller.");
    }

    props->setType(DLG_WARNING);
    props->setTitle(tr("Controller Preset Error"));
    props->setText(QString(tr("The preset for your controller \"%1\" is not "
                              "working properly."))
                           .arg(m_pController->getName()));
    props->setInfoText(QStringLiteral("<html>") +
            tr("The script code needs to be fixed.") + QStringLiteral("<p>") +
            additionalErrorText + QStringLiteral("</p></html>"));

    // Add "Details" text and set monospace font since they may contain
    // backtraces and code.
    props->setDetails(detailedError, true);

    // To prevent multiple windows for the same error
    props->setKey(key);

    // Allow user to suppress further notifications about this particular error
    if (!bFatalError) {
        props->addButton(QMessageBox::Ignore);
        props->addButton(QMessageBox::Retry);
    }
    props->addButton(QMessageBox::Close);
    props->setDefaultButton(QMessageBox::Close);
    props->setEscapeButton(QMessageBox::Close);
    props->setModal(false);

    if (ErrorDialogHandler::instance()->requestErrorDialog(props)) {
        // Enable custom handling of the dialog buttons
        connect(ErrorDialogHandler::instance(), SIGNAL(stdButtonClicked(QString, QMessageBox::StandardButton)),
                this, SLOT(errorDialogButton(QString, QMessageBox::StandardButton)));
    }
}

/* -------- ------------------------------------------------------
    Purpose: Slot to handle custom button clicks in error dialogs
    Input:   Key of dialog, StandardButton that was clicked
    Output:  -
    -------- ------------------------------------------------------ */
void ControllerEngine::errorDialogButton(const QString& key, QMessageBox::StandardButton button) {
    Q_UNUSED(key);

    // Something was clicked, so disable this signal now
    disconnect(ErrorDialogHandler::instance(),
               SIGNAL(stdButtonClicked(QString, QMessageBox::StandardButton)),
               this,
               SLOT(errorDialogButton(QString, QMessageBox::StandardButton)));

    if (button == QMessageBox::Retry) {
        emit resetController();
    }
}

ControlObjectScript* ControllerEngine::getControlObjectScript(const QString& group, const QString& name) {
    ConfigKey key = ConfigKey(group, name);

    if (!key.isValid()) {
        qWarning() << "ControllerEngine: Requested control with invalid key" << key;
        // Throw a debug assertion if controllerDebug is enabled
        DEBUG_ASSERT(!ControllerDebug::enabled());
        return nullptr;
    }

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

    if (coScript) {
        ControlObject* pControl = ControlObject::getControl(
                coScript->getKey(), onlyAssertOnControllerDebug());
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

    if (coScript) {
        ControlObject* pControl = ControlObject::getControl(
                coScript->getKey(), onlyAssertOnControllerDebug());
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
// Output:  a ScriptConnectionInvokableWrapper turned into a QtScriptValue.
//          The script should store this object to call its
//          'disconnect' and 'trigger' methods as needed.
//          If unsuccessful, returns undefined.
QScriptValue ControllerEngine::makeConnection(QString group, QString name,
                                              const QScriptValue callback) {
    VERIFY_OR_DEBUG_ASSERT(m_pEngine != nullptr) {
        qWarning() << "Tried to connect script callback, but there is no script engine!";
        return QScriptValue();
    }

    ControlObjectScript* coScript = getControlObjectScript(group, name);
    if (coScript == nullptr) {
        qWarning() << "ControllerEngine: script tried to connect to ControlObject (" +
                      group + ", " + name +
                      ") which is non-existent, ignoring.";
        return QScriptValue();
    }

    if (!callback.isFunction()) {
        qWarning() << "Tried to connect (" + group + ", " + name + ")"
                   << "to an invalid callback, ignoring.";
        return QScriptValue();
    }

    ScriptConnection connection;
    connection.key = ConfigKey(group, name);
    connection.controllerEngine = this;
    connection.callback = callback;
    connection.context = getThisObjectInFunctionCall();
    connection.id = QUuid::createUuid();

    if (coScript->addScriptConnection(connection)) {
        return m_pEngine->newQObject(
            new ScriptConnectionInvokableWrapper(connection),
            QScriptEngine::ScriptOwnership);
    }

    return QScriptValue();
}

/* -------- ------------------------------------------------------
   Purpose: Execute a ScriptConnection's callback
   Input:   the value of the connected ControlObject to pass to the callback
   -------- ------------------------------------------------------ */
void ScriptConnection::executeCallback(double value) const {
    QScriptValueList args;
    args << QScriptValue(value);
    args << QScriptValue(key.group);
    args << QScriptValue(key.item);
    QScriptValue func = callback; // copy function because QScriptValue::call is not const
    QScriptValue result = func.call(context, args);
    if (result.isError()) {
        qWarning() << "ControllerEngine: Invocation of connection " << id.toString()
                   << "connected to (" + key.group + ", " + key.item + ") failed:"
                   << result.toString();
    }
}

/* -------- ------------------------------------------------------
   Purpose: (Dis)connects a ScriptConnection
   Input:   the ScriptConnection to disconnect
   -------- ------------------------------------------------------ */
bool ControllerEngine::removeScriptConnection(const ScriptConnection connection) {
    ControlObjectScript* coScript = getControlObjectScript(connection.key.group,
                                                           connection.key.item);

    if (m_pEngine == nullptr || coScript == nullptr) {
        return false;
    }

    return coScript->removeScriptConnection(connection);
}

bool ScriptConnectionInvokableWrapper::disconnect() {
    // if the removeScriptConnection succeeded, the connection has been successfully disconnected
    bool success = m_scriptConnection.controllerEngine->removeScriptConnection(m_scriptConnection);
    m_isConnected = !success;
    return success;
}

/* -------- ------------------------------------------------------
   Purpose: Triggers the callback function of a ScriptConnection
   Input:   the ScriptConnection to trigger
   -------- ------------------------------------------------------ */
void ControllerEngine::triggerScriptConnection(const ScriptConnection connection) {
    if (m_pEngine == nullptr) {
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
QScriptValue ControllerEngine::connectControl(
        QString group, QString name, const QScriptValue passedCallback, bool disconnect) {
    // The passedCallback may or may not actually be a function, so when
    // the actual callback function is found, store it in this variable.
    QScriptValue actualCallbackFunction;

    if (passedCallback.isFunction()) {
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
        if (disconnect) {
            qWarning() << "ControllerEngine: script tried to disconnect from ControlObject (" +
                          group + ", " + name + ") which is non-existent, ignoring.";
        } else {
            qWarning() << "ControllerEngine: script tried to connect to ControlObject (" +
                           group + ", " + name + ") which is non-existent, ignoring.";
        }
        // This is inconsistent with other failures, which return false.
        // QScriptValue() with no arguments is undefined in JavaScript.
        return QScriptValue();
    }

    if (passedCallback.isString()) {
        // This check is redundant with makeConnection, but it must be done here
        // before evaluating the code string.
        VERIFY_OR_DEBUG_ASSERT(m_pEngine != nullptr) {
            qWarning() << "Tried to connect script callback, but there is no script engine!";
            return QScriptValue(false);
        }

        actualCallbackFunction = m_pEngine->evaluate(passedCallback.toString());

        if (checkException() || !actualCallbackFunction.isFunction()) {
            qWarning() << "Could not evaluate callback function:"
                        << passedCallback.toString();
            return QScriptValue(false);
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

            return m_pEngine->newQObject(
                new ScriptConnectionInvokableWrapper(connection),
                QScriptEngine::ScriptOwnership);
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
        return QScriptValue(false);
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
        return QScriptValue(true);
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
bool ControllerEngine::evaluate(const QFileInfo& scriptFile) {
    if (m_pEngine == nullptr) {
        return false;
    }

    if (!scriptFile.exists()) {
        qWarning() << "ControllerEngine: File does not exist:" << scriptFile.absoluteFilePath();
        return false;
    }
    m_scriptWatcher.addPath(scriptFile.absoluteFilePath());

    qDebug() << "ControllerEngine: Loading" << scriptFile.absoluteFilePath();

    // Read in the script file
    QString filename = scriptFile.absoluteFilePath();
    QFile input(filename);
    if (!input.open(QIODevice::ReadOnly)) {
        qWarning() << QString("ControllerEngine: Problem opening the script file: %1, error # %2, %3")
                .arg(filename, QString::number(input.error()), input.errorString());
        if (m_bPopups) {
            // Set up error dialog
            ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
            props->setType(DLG_WARNING);
            props->setTitle(tr("Controller Mapping File Problem"));
            props->setText(tr("The mapping for controller \"%1\" cannot be opened.").arg(m_pController->getName()));
            props->setInfoText(tr("The functionality provided by this controller mapping will be disabled until the issue has been resolved."));

            // We usually don't translate the details field, but the cause of
            // this problem lies in the user's system (e.g. a permission
            // issue). Translating this will help users to fix the issue even
            // when they don't speak english.
            props->setDetails(tr("File:") + QStringLiteral(" ") + filename +
                    QStringLiteral("\n") + tr("Error:") + QStringLiteral(" ") +
                    input.errorString());

            // Ask above layer to display the dialog & handle user response
            ErrorDialogHandler::instance()->requestErrorDialog(props);
        }
        return false;
    }

    QString scriptCode = "";
    scriptCode.append(input.readAll());
    scriptCode.append('\n');
    input.close();

    // Check syntax
    if (!syntaxIsValid(scriptCode, filename)) {
        return false;
    }

    // Evaluate the code
    QScriptValue scriptFunction = m_pEngine->evaluate(scriptCode, filename);

    // Record errors
    if (checkException(true)) {
        return false;
    }

    return true;
}

bool ControllerEngine::hasErrors(const QString& filename) {
    bool ret = m_scriptErrors.contains(filename);
    return ret;
}

/* -------- ------------------------------------------------------
   Purpose: Creates & starts a timer that runs some script code
                on timeout
   Input:   Number of milliseconds, script function to call,
                whether it should fire just once
   Output:  The timer's ID, 0 if starting it failed
   -------- ------------------------------------------------------ */
int ControllerEngine::beginTimer(int interval, QScriptValue timerCallback,
                                 bool oneShot) {
    if (!timerCallback.isFunction() && !timerCallback.isString()) {
        qWarning() << "Invalid timer callback provided to beginTimer."
                   << "Valid callbacks are strings and functions.";
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
    info.context = getThisObjectInFunctionCall();
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

    if (timerTarget.callback.isString()) {
        internalExecute(timerTarget.context, timerTarget.callback.toString());
    } else if (timerTarget.callback.isFunction()) {
        internalExecute(timerTarget.context, timerTarget.callback,
                        QScriptValueList());
    }
}

double ControllerEngine::getDeckRate(const QString& group) {
    double rate = 0.0;
    ControlObjectScript* pRateRatio = getControlObjectScript(group, "rate_ratio");
    if (pRateRatio != nullptr) {
        rate = pRateRatio->get();
    }

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
      scriptErrorDialog(error, error);
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
    if (m_dx[deck] != 0) {
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
    if (alpha != 0 && beta != 0) {
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
    ConfigKey key = ConfigKey(group, name);
    ControlObject* pControl = ControlObject::getControl(key, onlyAssertOnControllerDebug());
    if (!pControl) {
        qWarning() << "Failed to" << (set ? "enable" : "disable")
                   << "softTakeover for invalid control" << key;
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
void ControllerEngine::softTakeoverIgnoreNextValue(
        QString group, const QString name) {
    ConfigKey key = ConfigKey(group, name);
    ControlObject* pControl = ControlObject::getControl(key, onlyAssertOnControllerDebug());
    if (!pControl) {
        qWarning() << "Failed to call softTakeoverIgnoreNextValue for invalid control" << key;
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
