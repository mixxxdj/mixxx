#include "controllers/scripting/controllerscripthandler.h"

#include "control/controlobject.h"
#include "controllers/controller.h"
#include "controllers/controllerdebug.h"
#include "controllers/scripting/colormapperjsproxy.h"
#include "controllers/scripting/legacy/controllerscriptinterface.h"
#include "errordialoghandler.h"
#include "mixer/playermanager.h"

ControllerScriptHandler::ControllerScriptHandler(Controller* controller)
        : m_bDisplayingExceptionDialog(false),
          m_pScriptEngine(nullptr),
          m_pController(controller),
          m_bTesting(false) {
    // Handle error dialog buttons
    qRegisterMetaType<QMessageBox::StandardButton>("QMessageBox::StandardButton");

    initializeScriptEngine();
}

ControllerScriptHandler::~ControllerScriptHandler() {
    uninitializeScriptEngine();
}

bool ControllerScriptHandler::callFunctionOnObjects(QList<QString> scriptFunctionPrefixes,
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
            qWarning() << "ControllerScriptHandler: No" << prefixName << "object in script";
            continue;
        }

        QJSValue init = prefix.property(function);
        if (!init.isCallable()) {
            qWarning() << "ControllerScriptHandler:" << prefixName << "has no"
                       << function << " method";
            continue;
        }
        controllerDebug("ControllerScriptHandler: Executing"
                << prefixName << "." << function);
        QJSValue result = init.callWithInstance(prefix, args);
        if (result.isError()) {
            showScriptExceptionDialog(result, bFatalError);
            success = false;
        }
    }
    return success;
}

QJSValue ControllerScriptHandler::byteArrayToScriptValue(
        const QByteArray& byteArray) {
    // The QJSEngine converts the QByteArray to an ArrayBuffer object.
    QJSValue arrayBuffer = m_pScriptEngine->toScriptValue(byteArray);
    // Convert the ArrayBuffer to a Uint8 typed array so scripts can access its bytes
    // with the [] operator.
    QJSValue result =
            m_byteArrayToScriptValueJSFunction.call(QJSValueList{arrayBuffer});
    if (result.isError()) {
        showScriptExceptionDialog(result);
    }
    return result;
}

QJSValue ControllerScriptHandler::wrapFunctionCode(
        const QString& codeSnippet, int numberOfArgs) {
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

void ControllerScriptHandler::gracefulShutdown() {
    if (m_pScriptEngine == nullptr) {
        return;
    }

    qDebug() << "ControllerScriptHandler shutting down...";

    qDebug() << "Invoking shutdown() hook in scripts";
    callFunctionOnObjects(m_scriptFunctionPrefixes, "shutdown");

    if (m_shutdownFunction.isCallable()) {
        executeFunction(m_shutdownFunction, QJSValueList{});
    }

    qDebug() << "Clearing function wrapper cache";
    m_scriptWrappedFunctionCache.clear();
}

void ControllerScriptHandler::initializeScriptEngine() {
    VERIFY_OR_DEBUG_ASSERT(!m_pScriptEngine) {
        return;
    }

    // Create the Script Engine
    m_pScriptEngine = new QJSEngine(this);

    m_pScriptEngine->installExtensions(QJSEngine::ConsoleExtension);

    // Make this ControllerScriptHandler instance available to scripts as 'engine'.
    QJSValue engineGlobalObject = m_pScriptEngine->globalObject();
    ControllerScriptInterface* legacyScriptInterface =
            new ControllerScriptInterface(this);
    engineGlobalObject.setProperty(
            "engine", m_pScriptEngine->newQObject(legacyScriptInterface));

    QJSValue mapper = m_pScriptEngine->newQMetaObject(
            &ColorMapperJSProxy::staticMetaObject);
    engineGlobalObject.setProperty("ColorMapper", mapper);

    if (m_pController) {
        qDebug() << "Controller in script engine is:"
                 << m_pController->getName();

        ControllerJSProxy* controllerProxy = m_pController->jsProxy();

        // Make the Controller instance available to scripts
        engineGlobalObject.setProperty(
                "controller", m_pScriptEngine->newQObject(controllerProxy));

        // ...under the legacy name as well
        engineGlobalObject.setProperty(
                "midi", m_pScriptEngine->newQObject(controllerProxy));
    }

    m_byteArrayToScriptValueJSFunction = evaluateCodeString(
            "(function(arg1) { return new Uint8Array(arg1) })");
}

void ControllerScriptHandler::uninitializeScriptEngine() {
    // Delete the script engine, first clearing the pointer so that
    // other threads will not get the dead pointer after we delete it.
    if (m_pScriptEngine != nullptr) {
        QJSEngine* engine = m_pScriptEngine;
        m_pScriptEngine = nullptr;
        engine->deleteLater();
    }
}

void ControllerScriptHandler::loadModule(QFileInfo moduleFileInfo) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    m_moduleFileInfo = moduleFileInfo;

    QJSValue mod =
            m_pScriptEngine->importModule(moduleFileInfo.absoluteFilePath());
    if (mod.isError()) {
        showScriptExceptionDialog(mod);
        return;
    }

    connect(&m_scriptWatcher,
            &QFileSystemWatcher::fileChanged,
            this,
            &ControllerScriptHandler::scriptHasChanged);
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

void ControllerScriptHandler::handleInput(
        QByteArray data, mixxx::Duration timestamp) {
    if (m_handleInputFunction.isCallable()) {
        QJSValueList args;
        args << byteArrayToScriptValue(data);
        args << timestamp.toDoubleMillis();
        executeFunction(m_handleInputFunction, args);
    }
}

bool ControllerScriptHandler::loadScriptFiles(
        const QList<ControllerPreset::ScriptFileInfo>& scripts) {
    bool scriptsEvaluatedCorrectly = true;
    for (const auto& script : scripts) {
        if (!evaluateScriptFile(script.file)) {
            scriptsEvaluatedCorrectly = false;
        }
    }

    m_lastScriptFiles = scripts;

    connect(&m_scriptWatcher,
            &QFileSystemWatcher::fileChanged,
            this,
            &ControllerScriptHandler::scriptHasChanged);

    if (!scriptsEvaluatedCorrectly) {
        gracefulShutdown();
        uninitializeScriptEngine();
    }

    return scriptsEvaluatedCorrectly;
}

void ControllerScriptHandler::scriptHasChanged(const QString& scriptFilename) {
    Q_UNUSED(scriptFilename);
    disconnect(&m_scriptWatcher,
            &QFileSystemWatcher::fileChanged,
            this,
            &ControllerScriptHandler::scriptHasChanged);
    reloadScripts();
}

void ControllerScriptHandler::reloadScripts() {
    qDebug() << "ControllerScriptHandler: Reloading Scripts";
    ControllerPresetPointer pPreset = m_pController->getPreset();

    gracefulShutdown();
    uninitializeScriptEngine();

    initializeScriptEngine();
    if (!loadScriptFiles(m_lastScriptFiles)) {
        return;
    }

    qDebug() << "Re-initializing scripts";
    initializeScripts(m_lastScriptFiles);
    loadModule(m_moduleFileInfo);
}

void ControllerScriptHandler::initializeScripts(
        const QList<ControllerPreset::ScriptFileInfo>& scripts) {
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
    bool success =
            callFunctionOnObjects(m_scriptFunctionPrefixes, "init", args, true);

    // We failed to initialize the controller scripts, shutdown the script
    // engine to avoid error popups on every button press or slider move
    if (!success) {
        gracefulShutdown();
        uninitializeScriptEngine();
    }
}

bool ControllerScriptHandler::executeFunction(
        QJSValue functionObject, QJSValueList args) {
    // This function is called from outside the controller engine, so we can't
    // use VERIFY_OR_DEBUG_ASSERT here
    if (!m_pScriptEngine) {
        return false;
    }

    if (functionObject.isError()) {
        qDebug() << "ControllerScriptHandler::executeFunction:"
                 << functionObject.toString();
        return false;
    }

    // If it's not a function, we're done.
    if (!functionObject.isCallable()) {
        qDebug() << "ControllerScriptHandler::executeFunction:"
                 << functionObject.toVariant() << "Not a function";
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

bool ControllerScriptHandler::executeFunction(
        QJSValue functionObject, const QByteArray& data) {
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

QJSValue ControllerScriptHandler::evaluateCodeString(
        const QString& program, const QString& fileName, int lineNumber) {
    VERIFY_OR_DEBUG_ASSERT(m_pScriptEngine) {
        return QJSValue::UndefinedValue;
    }
    return m_pScriptEngine->evaluate(program, fileName, lineNumber);
}

void ControllerScriptHandler::throwJSError(const QString& message) {
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

void ControllerScriptHandler::showScriptExceptionDialog(
        QJSValue evaluationResult, bool bFatalError) {
    VERIFY_OR_DEBUG_ASSERT(evaluationResult.isError()) {
        return;
    }

    QString errorMessage = evaluationResult.toString();
    QString line = evaluationResult.property("lineNumber").toString();
    QString backtrace = evaluationResult.property("stack").toString();
    QString filename = evaluationResult.property("fileName").toString();

    QString errorText;
    if (filename.isEmpty()) {
        errorText = QString("Uncaught exception at line %1 in passed code.")
                            .arg(line);
    } else {
        errorText = QString("Uncaught exception at line %1 in file %2.")
                            .arg(line, filename);
    }

    errorText += QStringLiteral("\n\nException:\n  ") + errorMessage;

    // Do not include backtrace in dialog key because it might contain midi
    // slider values that will differ most of the time. This would break
    // the "Ignore" feature of the error dialog.
    QString key = errorText;
    qWarning() << "ControllerScriptHandler:" << errorText;

    // Add backtrace to the error details
    errorText += QStringLiteral("\n\nBacktrace:\n") + backtrace;

    if (!m_bDisplayingExceptionDialog) {
        scriptErrorDialog(errorText, key, bFatalError);
    }
}

void ControllerScriptHandler::scriptErrorDialog(
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
        connect(ErrorDialogHandler::instance(),
                &ErrorDialogHandler::stdButtonClicked,
                this,
                &ControllerScriptHandler::errorDialogButton);
    }
}

void ControllerScriptHandler::errorDialogButton(
        const QString& key, QMessageBox::StandardButton clickedButton) {
    Q_UNUSED(key);

    m_bDisplayingExceptionDialog = false;
    // Something was clicked, so disable this signal now
    disconnect(ErrorDialogHandler::instance(),
            &ErrorDialogHandler::stdButtonClicked,
            this,
            &ControllerScriptHandler::errorDialogButton);

    if (clickedButton == QMessageBox::Retry) {
        reloadScripts();
    }
}

bool ControllerScriptHandler::evaluateScriptFile(const QFileInfo& scriptFile) {
    VERIFY_OR_DEBUG_ASSERT(m_pScriptEngine) {
        return false;
    }

    if (!scriptFile.exists()) {
        qWarning() << "ControllerScriptHandler: File does not exist:"
                   << scriptFile.absoluteFilePath();
        return false;
    }
    m_scriptWatcher.addPath(scriptFile.absoluteFilePath());

    qDebug() << "ControllerScriptHandler: Loading"
             << scriptFile.absoluteFilePath();

    // Read in the script file
    QString filename = scriptFile.absoluteFilePath();
    QFile input(filename);
    if (!input.open(QIODevice::ReadOnly)) {
        qWarning() << QString(
                "ControllerScriptHandler: Problem opening the script file: %1, "
                "error # %2, %3")
                              .arg(filename,
                                      QString::number(input.error()),
                                      input.errorString());
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
