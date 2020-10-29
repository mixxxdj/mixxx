#include "controllers/engine/controllerengine.h"

#include "control/controlobject.h"
#include "control/controlobjectscript.h"
#include "controllers/controller.h"
#include "controllers/controllerdebug.h"
#include "controllers/engine/colormapperjsproxy.h"
#include "controllers/engine/controllerenginejsproxy.h"
#include "controllers/engine/scriptconnectionjsproxy.h"
#include "errordialoghandler.h"
#include "mixer/playermanager.h"
// to tell the msvs compiler about `isnan`
#include "util/math.h"
#include "util/time.h"

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

    uninitializeScriptEngine();
}

bool ControllerEngine::callFunctionOnObjects(QList<QString> scriptFunctionPrefixes,
        const QString& function,
        QJSValueList args,
        bool bFatalError) {
    VERIFY_OR_DEBUG_ASSERT(m_pScriptEngine) {
        return false;
    }

    const QJSValue global = m_pScriptEngine->globalObject();

    bool success = true;
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
            showScriptExceptionDialog(result, bFatalError);
            success = false;
        }
    }
    return success;
}

QJSValue ControllerEngine::byteArrayToScriptValue(const QByteArray& byteArray) {
    // The QJSEngine converts the QByteArray to an ArrayBuffer object.
    QJSValue arrayBuffer = m_pScriptEngine->toScriptValue(byteArray);
    // Convert the ArrayBuffer to a Uint8 typed array so scripts can access its bytes
    // with the [] operator.
    QJSValue result = m_byteArrayToScriptValueJSFunction.call(
            QJSValueList{arrayBuffer});
    if (result.isError()) {
        showScriptExceptionDialog(result);
    }
    return result;
}

QJSValue ControllerEngine::wrapFunctionCode(const QString& codeSnippet,
        int numberOfArgs) {
    // This function is called from outside the controller engine, so we can't
    // use VERIFY_OR_DEBUG_ASSERT here
    if (m_pScriptEngine == nullptr) {
        return QJSValue();
    }

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
        QString wrappedCode = QStringLiteral("(function (") + wrapperArgs +
                QStringLiteral(") { (") + codeSnippet + QStringLiteral(")(") +
                wrapperArgs + QStringLiteral("); })");

        wrappedFunction = evaluateCodeString(wrappedCode);
        if (wrappedFunction.isError()) {
            showScriptExceptionDialog(wrappedFunction);
        }
        m_scriptWrappedFunctionCache[codeSnippet] = wrappedFunction;
    }
    return wrappedFunction;
}

void ControllerEngine::gracefulShutdown() {
    if (m_pScriptEngine == nullptr) {
        return;
    }

    qDebug() << "ControllerEngine shutting down...";

    // Stop all timers
    stopAllTimers();

    qDebug() << "Invoking shutdown() hook in scripts";
    callFunctionOnObjects(m_scriptFunctionPrefixes, "shutdown");

    if (m_shutdownFunction.isCallable()) {
        executeFunction(m_shutdownFunction, QJSValueList{});
    }

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
}

void ControllerEngine::initializeScriptEngine() {
    VERIFY_OR_DEBUG_ASSERT(!m_pScriptEngine) {
        return;
    }

    // Create the Script Engine
    m_pScriptEngine = new QJSEngine(this);

    m_pScriptEngine->installExtensions(QJSEngine::ConsoleExtension);

    // Make this ControllerEngine instance available to scripts as 'engine'.
    QJSValue engineGlobalObject = m_pScriptEngine->globalObject();
    ControllerEngineJSProxy* proxy = new ControllerEngineJSProxy(this);
    engineGlobalObject.setProperty("engine", m_pScriptEngine->newQObject(proxy));

    QJSValue mapper = m_pScriptEngine->newQMetaObject(&ColorMapperJSProxy::staticMetaObject);
    engineGlobalObject.setProperty("ColorMapper", mapper);

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

void ControllerEngine::uninitializeScriptEngine() {
    // Delete the script engine, first clearing the pointer so that
    // other threads will not get the dead pointer after we delete it.
    if (m_pScriptEngine != nullptr) {
        QJSEngine* engine = m_pScriptEngine;
        m_pScriptEngine = nullptr;
        engine->deleteLater();
    }
}

void ControllerEngine::loadModule(QFileInfo moduleFileInfo) {
    // QFileInfo does not have a isValid/isEmpty/isNull method to check if it
    // actually contains a reference, so we check if the filePath is empty as a
    // workaround.
    // See https://stackoverflow.com/a/45652741/1455128 for details.
    VERIFY_OR_DEBUG_ASSERT(!moduleFileInfo.filePath().isEmpty()) {
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(moduleFileInfo.isFile()) {
        return;
    }
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    m_moduleFileInfo = moduleFileInfo;

    QJSValue mod = m_pScriptEngine->importModule(moduleFileInfo.absoluteFilePath());
    if (mod.isError()) {
        showScriptExceptionDialog(mod);
        return;
    }

    connect(&m_scriptWatcher,
            &QFileSystemWatcher::fileChanged,
            this,
            &ControllerEngine::scriptHasChanged);
    m_scriptWatcher.addPath(moduleFileInfo.absoluteFilePath());

    QJSValue initFunction = mod.property("init");
    executeFunction(initFunction, QJSValueList{});

    QJSValue handleInputFunction = mod.property("handleInput");
    if (handleInputFunction.isCallable()) {
        m_handleInputFunction = handleInputFunction;
    } else {
        scriptErrorDialog(
                "Controller JavaScript module exports no handleInput function.",
                QStringLiteral("handleInput"),
                true);
    }

    QJSValue shutdownFunction = mod.property("shutdown");
    if (shutdownFunction.isCallable()) {
        m_shutdownFunction = shutdownFunction;
    } else {
        qDebug() << "Module exports no shutdown function.";
    }
#else
    Q_UNUSED(moduleFileInfo);
#endif
}

void ControllerEngine::handleInput(QByteArray data, mixxx::Duration timestamp) {
    if (m_handleInputFunction.isCallable()) {
        QJSValueList args;
        args << byteArrayToScriptValue(data);
        args << timestamp.toDoubleMillis();
        executeFunction(m_handleInputFunction, args);
    }
}

bool ControllerEngine::loadScriptFiles(const QList<ControllerPreset::ScriptFileInfo>& scripts) {
    bool scriptsEvaluatedCorrectly = true;
    for (const auto& script : scripts) {
        if (!evaluateScriptFile(script.file)) {
            scriptsEvaluatedCorrectly = false;
        }
    }

    m_lastScriptFiles = scripts;

    connect(&m_scriptWatcher, &QFileSystemWatcher::fileChanged, this, &ControllerEngine::scriptHasChanged);

    if (!scriptsEvaluatedCorrectly) {
        gracefulShutdown();
        uninitializeScriptEngine();
    }

    return scriptsEvaluatedCorrectly;
}

void ControllerEngine::scriptHasChanged(const QString& scriptFilename) {
    Q_UNUSED(scriptFilename);
    disconnect(&m_scriptWatcher, &QFileSystemWatcher::fileChanged, this, &ControllerEngine::scriptHasChanged);
    reloadScripts();
}

void ControllerEngine::reloadScripts() {
    qDebug() << "ControllerEngine: Reloading Scripts";
    ControllerPresetPointer pPreset = m_pController->getPreset();

    gracefulShutdown();
    uninitializeScriptEngine();

    initializeScriptEngine();
    if (!loadScriptFiles(m_lastScriptFiles)) {
        return;
    }

    qDebug() << "Re-initializing scripts";
    initializeScripts(m_lastScriptFiles);

    // QFileInfo does not have a isValid/isEmpty/isNull method to check if it
    // actually contains a reference, so we check if the filePath is empty as a
    // workaround.
    // See https://stackoverflow.com/a/45652741/1455128 for details.
    if (!m_moduleFileInfo.filePath().isEmpty()) {
        loadModule(m_moduleFileInfo);
    }
}

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
    bool success = callFunctionOnObjects(m_scriptFunctionPrefixes, "init", args, true);

    // We failed to initialize the controller scripts, shutdown the script
    // engine to avoid error popups on every button press or slider move
    if (!success) {
        gracefulShutdown();
        uninitializeScriptEngine();
    }
}

bool ControllerEngine::executeFunction(QJSValue functionObject, QJSValueList args) {
    // This function is called from outside the controller engine, so we can't
    // use VERIFY_OR_DEBUG_ASSERT here
    if (!m_pScriptEngine) {
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

bool ControllerEngine::executeFunction(QJSValue functionObject, const QByteArray& data) {
    // This function is called from outside the controller engine, so we can't
    // use VERIFY_OR_DEBUG_ASSERT here
    if (!m_pScriptEngine) {
        return false;
    }
    QJSValueList args;
    args << byteArrayToScriptValue(data);
    args << QJSValue(data.size());
    return executeFunction(functionObject, args);
}

QJSValue ControllerEngine::evaluateCodeString(const QString& program, const QString& fileName, int lineNumber) {
    VERIFY_OR_DEBUG_ASSERT(m_pScriptEngine) {
        return QJSValue::UndefinedValue;
    }
    return m_pScriptEngine->evaluate(program, fileName, lineNumber);
}

void ControllerEngine::throwJSError(const QString& message) {
#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
    QString errorText = tr("Uncaught exception: %1").arg(message);
    qWarning() << "ControllerEngine:" << errorText;
    if (!m_bDisplayingExceptionDialog) {
        scriptErrorDialog(errorText, errorText);
    }
#else
    m_pScriptEngine->throwError(message);
#endif
}

void ControllerEngine::showScriptExceptionDialog(QJSValue evaluationResult, bool bFatalError) {
    VERIFY_OR_DEBUG_ASSERT(evaluationResult.isError()) {
        return;
    }

    QString errorMessage = evaluationResult.toString();
    QString line = evaluationResult.property("lineNumber").toString();
    QString backtrace = evaluationResult.property("stack").toString();
    QString filename = evaluationResult.property("fileName").toString();

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
    qWarning() << "ControllerEngine:" << errorText;

    // Add backtrace to the error details
    errorText += QStringLiteral("\n\nBacktrace:\n") + backtrace;

    if (!m_bDisplayingExceptionDialog) {
        scriptErrorDialog(errorText, key, bFatalError);
    }
}

void ControllerEngine::scriptErrorDialog(
        const QString& detailedError, const QString& key, bool bFatalError) {
    if (m_bTesting) {
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
        props->setDefaultButton(QMessageBox::Ignore);
        props->setEscapeButton(QMessageBox::Ignore);
    } else {
        props->addButton(QMessageBox::Close);
        props->setDefaultButton(QMessageBox::Close);
        props->setEscapeButton(QMessageBox::Close);
    }
    props->setModal(false);

    if (ErrorDialogHandler::instance()->requestErrorDialog(props)) {
        m_bDisplayingExceptionDialog = true;
        // Enable custom handling of the dialog buttons
        connect(ErrorDialogHandler::instance(), &ErrorDialogHandler::stdButtonClicked, this, &ControllerEngine::errorDialogButton);
    }
}

void ControllerEngine::errorDialogButton(
        const QString& key, QMessageBox::StandardButton clickedButton) {
    Q_UNUSED(key);

    m_bDisplayingExceptionDialog = false;
    // Something was clicked, so disable this signal now
    disconnect(
            ErrorDialogHandler::instance(),
            &ErrorDialogHandler::stdButtonClicked,
            this,
            &ControllerEngine::errorDialogButton);

    if (clickedButton == QMessageBox::Retry) {
        reloadScripts();
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

double ControllerEngine::getValue(QString group, QString name) {
    ControlObjectScript* coScript = getControlObjectScript(group, name);
    if (coScript == nullptr) {
        qWarning() << "ControllerEngine: Unknown control" << group << name << ", returning 0.0";
        return 0.0;
    }
    return coScript->get();
}

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

double ControllerEngine::getParameter(QString group, QString name) {
    ControlObjectScript* coScript = getControlObjectScript(group, name);
    if (coScript == nullptr) {
        qWarning() << "ControllerEngine: Unknown control" << group << name << ", returning 0.0";
        return 0.0;
    }
    return coScript->getParameter();
}

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

void ControllerEngine::reset(QString group, QString name) {
    ControlObjectScript* coScript = getControlObjectScript(group, name);
    if (coScript != nullptr) {
        coScript->reset();
    }
}

double ControllerEngine::getDefaultValue(QString group, QString name) {
    ControlObjectScript* coScript = getControlObjectScript(group, name);

    if (coScript == nullptr) {
        qWarning() << "ControllerEngine: Unknown control" << group << name << ", returning 0.0";
        return 0.0;
    }

    return coScript->getDefault();
}

double ControllerEngine::getDefaultParameter(QString group, QString name) {
    ControlObjectScript* coScript = getControlObjectScript(group, name);

    if (coScript == nullptr) {
        qWarning() << "ControllerEngine: Unknown control" << group << name << ", returning 0.0";
        return 0.0;
    }

    return coScript->getParameterForValue(coScript->getDefault());
}

void ControllerEngine::log(QString message) {
    controllerDebug(message);
}

QJSValue ControllerEngine::makeConnection(QString group, QString name, const QJSValue callback) {
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
        throwJSError("Tried to connect (" + group + ", " + name + ")" + " to an invalid callback. Make sure that your code contains no syntax errors.");
        return QJSValue();
    }

    ScriptConnection connection;
    connection.key = ConfigKey(group, name);
    connection.controllerEngine = this;
    connection.callback = callback;
    connection.id = QUuid::createUuid();

    if (coScript->addScriptConnection(connection)) {
        return m_pScriptEngine->newQObject(new ScriptConnectionJSProxy(connection));
    }

    return QJSValue();
}

bool ControllerEngine::removeScriptConnection(const ScriptConnection connection) {
    ControlObjectScript* coScript = getControlObjectScript(connection.key.group,
            connection.key.item);

    if (m_pScriptEngine == nullptr || coScript == nullptr) {
        return false;
    }

    return coScript->removeScriptConnection(connection);
}

void ControllerEngine::triggerScriptConnection(const ScriptConnection connection) {
    VERIFY_OR_DEBUG_ASSERT(m_pScriptEngine) {
        return;
    }

    ControlObjectScript* coScript = getControlObjectScript(connection.key.group,
            connection.key.item);
    if (coScript == nullptr) {
        return;
    }

    connection.executeCallback(coScript->get());
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
        QString group, QString name, QJSValue passedCallback, bool disconnect) {
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

            return m_pScriptEngine->newQObject(new ScriptConnectionJSProxy(connection));
        }
    } else if (passedCallback.isQObject()) {
        // Assume a ScriptConnection and assume that the script author
        // wants to disconnect it, regardless of the disconnect parameter
        // and regardless of whether it is connected to the same ControlObject
        // specified by the first two parameters to this function.
        QObject* qobject = passedCallback.toQObject();
        const QMetaObject* qmeta = qobject->metaObject();

        qWarning() << "QObject passed to engine.connectControl. Assuming it is"
                   << "a connection object to disconnect and returning false.";
        if (!strcmp(qmeta->className(),
                    "ScriptConnectionJSProxy")) {
            ScriptConnectionJSProxy* proxy =
                    (ScriptConnectionJSProxy*)qobject;
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

void ControllerEngine::trigger(QString group, QString name) {
    ControlObjectScript* coScript = getControlObjectScript(group, name);
    if (coScript != nullptr) {
        coScript->emitValueChanged();
    }
}

bool ControllerEngine::evaluateScriptFile(const QFileInfo& scriptFile) {
    VERIFY_OR_DEBUG_ASSERT(m_pScriptEngine) {
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
        // Set up error dialog
        ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
        props->setType(DLG_WARNING);
        props->setTitle(tr("Controller Mapping File Problem"));
        props->setText(tr("The mapping for controller \"%1\" cannot be opened.")
                               .arg(m_pController->getName()));
        props->setInfoText(
                tr("The functionality provided by this controller mapping will "
                   "be disabled until the issue has been resolved."));

        // We usually don't translate the details field, but the cause of
        // this problem lies in the user's system (e.g. a permission
        // issue). Translating this will help users to fix the issue even
        // when they don't speak english.
        props->setDetails(tr("File:") + QStringLiteral(" ") + filename +
                QStringLiteral("\n") + tr("Error:") + QStringLiteral(" ") +
                input.errorString());

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
        showScriptExceptionDialog(scriptFunction, true);
        return false;
    }

    return true;
}

int ControllerEngine::beginTimer(int intervalMillis, QJSValue timerCallback, bool oneShot) {
    if (timerCallback.isString()) {
        timerCallback = evaluateCodeString(timerCallback.toString());
    } else if (!timerCallback.isCallable()) {
        QString sErrorMessage(
                "Invalid timer callback provided to engine.beginTimer. Valid callbacks are strings and functions. "
                "Make sure that your code contains no syntax errors.");
        if (timerCallback.isError()) {
            sErrorMessage.append("\n" + timerCallback.toString());
        }
        throwJSError(sErrorMessage);
        return 0;
    }

    if (intervalMillis < 20) {
        qWarning() << "Timer request for" << intervalMillis
                   << "ms is too short. Setting to the minimum of 20ms.";
        intervalMillis = 20;
    }

    // This makes use of every QObject's internal timer mechanism. Nice, clean,
    // and simple. See http://doc.trolltech.com/4.6/qobject.html#startTimer for
    // details
    int timerId = startTimer(intervalMillis);
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

void ControllerEngine::timerEvent(QTimerEvent* event) {
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

void ControllerEngine::softTakeoverIgnoreNextValue(QString group, const QString name) {
    ConfigKey key = ConfigKey(group, name);
    ControlObject* pControl = ControlObject::getControl(key, onlyAssertOnControllerDebug());
    if (!pControl) {
        qWarning() << "Failed to call softTakeoverIgnoreNextValue for invalid control" << key;
        return;
    }

    m_st.ignoreNext(pControl);
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

void ControllerEngine::scratchEnable(
        int deck,
        int intervalsPerRev,
        double rpm,
        double alpha,
        double beta,
        bool ramp) {
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

void ControllerEngine::scratchTick(int deck, int interval) {
    m_lastMovement[deck] = mixxx::Time::elapsed();
    m_intervalAccumulator[deck] += interval;
}

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
        filter->observation(m_rampTo[deck] * kAlphaBetaDt);
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
            ((m_brakeActive[deck] || m_softStartActive[deck]) && ((oldRate > m_rampTo[deck] && newRate < m_rampTo[deck]) || (oldRate < m_rampTo[deck] && newRate > m_rampTo[deck]))) ||
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
    m_ramp[deck] = true; // Activate the ramping in scratchProcess()
}

bool ControllerEngine::isScratching(int deck) {
    // PlayerManager::groupForDeck is 0-indexed.
    QString group = PlayerManager::groupForDeck(deck - 1);
    return getValue(group, "scratch2_enable") > 0;
}

void ControllerEngine::spinback(int deck, bool activate, double factor, double rate) {
    // defaults for args set in header file
    brake(deck, activate, factor, rate);
}

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
        if (initRate == 1.0) { // then rate is really 1.0 or was set to default
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
        double alphaBrake = 1.0 / 512;
        // avoid decimals for fine adjusting
        if (factor > 1) {
            factor = ((factor - 1) / 10) + 1;
        }
        double betaBrake = ((1.0 / 512) / 1024) * factor; // default*factor
        AlphaBetaFilter* filter = m_scratchFilters[deck];
        if (filter != nullptr) {
            filter->init(kAlphaBetaDt, initRate, alphaBrake, betaBrake);
        }

        // activate the ramping in scratchProcess()
        m_ramp[deck] = true;
    }
}

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
        double alphaSoft = 1.0 / 512;
        // avoid decimals for fine adjusting
        if (factor > 1) {
            factor = ((factor - 1) / 10) + 1;
        }
        double betaSoft = ((1.0 / 512) / 1024) * factor; // default: (1.0/512)/1024
        AlphaBetaFilter* filter = m_scratchFilters[deck];
        if (filter != nullptr) { // kAlphaBetaDt = 1/1000 seconds
            filter->init(kAlphaBetaDt, initRate, alphaSoft, betaSoft);
        }

        // activate the ramping in scratchProcess()
        m_ramp[deck] = true;
    }
}
