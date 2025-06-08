#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"

#include "control/controlobject.h"
#include "controllers/controller.h"
#include "controllers/scripting/colormapperjsproxy.h"
#include "controllers/scripting/legacy/controllerscriptinterfacelegacy.h"
#include "errordialoghandler.h"
#include "mixer/playermanager.h"
#include "moc_controllerscriptenginelegacy.cpp"

ControllerScriptEngineLegacy::ControllerScriptEngineLegacy(
        Controller* controller, const RuntimeLoggingCategory& logger)
        : ControllerScriptEngineBase(controller, logger) {
    connect(&m_fileWatcher,
            &QFileSystemWatcher::fileChanged,
            this,
            &ControllerScriptEngineLegacy::reload);
}

ControllerScriptEngineLegacy::~ControllerScriptEngineLegacy() {
    shutdown();
}

bool ControllerScriptEngineLegacy::callFunctionOnObjects(
        const QList<QString>& scriptFunctionPrefixes,
        const QString& function,
        const QJSValueList& args,
        bool bFatalError) {
    VERIFY_OR_DEBUG_ASSERT(m_pJSEngine) {
        return false;
    }

    const QJSValue global = m_pJSEngine->globalObject();

    bool success = true;
    for (const QString& prefixName : scriptFunctionPrefixes) {
        QJSValue prefix = global.property(prefixName);
        if (!prefix.isObject()) {
            qCWarning(m_logger) << "No" << prefixName << "object in script";
            continue;
        }

        QJSValue init = prefix.property(function);
        if (!init.isCallable()) {
            qCWarning(m_logger) << prefixName << "has no"
                                << function << " method";
            continue;
        }
        qCDebug(m_logger) << "Executing"
                          << prefixName << "." << function;
        QJSValue result = init.callWithInstance(prefix, args);
        if (result.isError()) {
            showScriptExceptionDialog(result, bFatalError);
            success = false;
        }
    }
    return success;
}

QJSValue ControllerScriptEngineLegacy::wrapFunctionCode(
        const QString& codeSnippet, int numberOfArgs) {
    // This function is called from outside the controller engine, so we can't
    // use VERIFY_OR_DEBUG_ASSERT here
    if (!m_pJSEngine) {
        return QJSValue();
    }

    QJSValue wrappedFunction;

    const auto it = m_scriptWrappedFunctionCache.constFind(codeSnippet);
    if (it != m_scriptWrappedFunctionCache.constEnd()) {
        wrappedFunction = it.value();
    } else {
        QStringList wrapperArgList;
        wrapperArgList.reserve(numberOfArgs);
        for (int i = 1; i <= numberOfArgs; i++) {
            wrapperArgList << QString("arg%1").arg(i);
        }
        QString wrapperArgs = wrapperArgList.join(",");
        QString wrappedCode = QStringLiteral("(function (") + wrapperArgs +
                QStringLiteral(") { (") + codeSnippet + QStringLiteral(")(") +
                wrapperArgs + QStringLiteral("); })");

        wrappedFunction = m_pJSEngine->evaluate(wrappedCode);
        if (wrappedFunction.isError()) {
            showScriptExceptionDialog(wrappedFunction);
        }
        m_scriptWrappedFunctionCache[codeSnippet] = wrappedFunction;
    }
    return wrappedFunction;
}

void ControllerScriptEngineLegacy::setScriptFiles(
        QList<LegacyControllerMapping::ScriptFileInfo> scripts) {
    const QStringList paths = m_fileWatcher.files();
    if (!paths.isEmpty()) {
        m_fileWatcher.removePaths(paths);
    }
    m_scriptFiles = std::move(scripts);
}

void ControllerScriptEngineLegacy::setSettings(
        const QList<std::shared_ptr<AbstractLegacyControllerSetting>>& settings) {
    m_settings.clear();
    for (const auto& pSetting : std::as_const(settings)) {
        QString name = pSetting->variableName();
        VERIFY_OR_DEBUG_ASSERT(!name.isEmpty()) {
            continue;
        }
        m_settings[name] = pSetting->value();
    }
}

bool ControllerScriptEngineLegacy::initialize() {
    if (!ControllerScriptEngineBase::initialize()) {
        return false;
    }

    // Binary data is passed from the Controller as a QByteArray, which
    // QJSEngine::toScriptValue converts to an ArrayBuffer in JavaScript.
    // ArrayBuffer cannot be accessed with the [] operator in JS; it needs
    // to be converted to a typed array (Uint8Array in this case) first.
    // This function generates a wrapper function from a JS callback to do
    // that conversion automatically.
    m_makeArrayBufferWrapperFunction = m_pJSEngine->evaluate(QStringLiteral(
            // arg2 is the timestamp for ControllerScriptModuleEngine.
            // In ControllerScriptEngineLegacy it is the length of the array.
            "(function(callback) {"
            "    return function(arrayBuffer, arg2) {"
            "        callback(new Uint8Array(arrayBuffer), arg2);"
            "    };"
            "})"));

    // Make this ControllerScriptHandler instance available to scripts as 'engine'.
    QJSValue engineGlobalObject = m_pJSEngine->globalObject();
    ControllerScriptInterfaceLegacy* legacyScriptInterface =
            new ControllerScriptInterfaceLegacy(this, m_logger);

    engineGlobalObject.setProperty(
            "engine", m_pJSEngine->newQObject(legacyScriptInterface));

    for (const LegacyControllerMapping::ScriptFileInfo& script : std::as_const(m_scriptFiles)) {
        if (!evaluateScriptFile(script.file)) {
            shutdown();
            return false;
        }
        if (!script.functionPrefix.isEmpty()) {
            m_scriptFunctionPrefixes.append(script.functionPrefix);
        }
    }

    // For testing, do not actually initialize the scripts, just check for
    // syntax errors above.
    if (m_bTesting) {
        return true;
    }

    for (QString functionName : std::as_const(m_scriptFunctionPrefixes)) {
        if (functionName.isEmpty()) {
            continue;
        }
        functionName.append(QStringLiteral(".incomingData"));
        m_incomingDataFunctions.append(
                wrapArrayBufferCallback(
                        wrapFunctionCode(functionName, 2)));
    }

    // m_pController is nullptr in tests.
    const auto controllerName = m_pController ? m_pController->getName() : QString{};
    const auto args = QJSValueList{
            controllerName,
            m_logger().isDebugEnabled(),
    };
    if (!callFunctionOnObjects(m_scriptFunctionPrefixes, "init", args, true)) {
        shutdown();
        return false;
    }

    return true;
}

void ControllerScriptEngineLegacy::shutdown() {
    // There is no js engine if the mapping was not loaded from a file but by
    // creating a new, empty mapping LegacyMidiControllerMapping with the wizard
    if (m_pJSEngine) {
        callFunctionOnObjects(m_scriptFunctionPrefixes, "shutdown");
    }
    m_scriptWrappedFunctionCache.clear();
    m_incomingDataFunctions.clear();
    m_scriptFunctionPrefixes.clear();
    if (m_pJSEngine) {
        ControllerScriptEngineBase::shutdown();
    }
}

bool ControllerScriptEngineLegacy::handleIncomingData(const QByteArray& data) {
    // This function is called from outside the controller engine, so we can't
    // use VERIFY_OR_DEBUG_ASSERT here
    if (!m_pJSEngine) {
        return false;
    }

    const auto args = QJSValueList{
            m_pJSEngine->toScriptValue(data),
            static_cast<uint>(data.size()),
    };

    for (auto&& function : m_incomingDataFunctions) {
        ControllerScriptEngineBase::executeFunction(&function, args);
    }

    return true;
}

bool ControllerScriptEngineLegacy::evaluateScriptFile(const QFileInfo& scriptFile) {
    VERIFY_OR_DEBUG_ASSERT(m_pJSEngine) {
        return false;
    }

    if (!scriptFile.exists()) {
        qCWarning(m_logger) << "File does not exist:"
                            << scriptFile.absoluteFilePath();
        return false;
    }

    // If the script is invalid, it should be watched so the user can fix it
    // without having to restart Mixxx. So, add it to the watcher before
    // evaluating it.
    if (!m_fileWatcher.addPath(scriptFile.absoluteFilePath())) {
        qCWarning(m_logger) << "Failed to watch script file" << scriptFile.absoluteFilePath();
    };

    qCDebug(m_logger) << "Loading"
                      << scriptFile.absoluteFilePath();

    // Read in the script file
    QString filename = scriptFile.absoluteFilePath();
    QFile input(filename);
    if (!input.open(QIODevice::ReadOnly)) {
        qCWarning(m_logger) << QString(
                "Problem opening the script file: %1, "
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
                        input.errorString(),
                true /* use monospace font / expand Details box */);

        // Ask above layer to display the dialog & handle user response
        ErrorDialogHandler::instance()->requestErrorDialog(props);
        return false;
    }

    QString scriptCode = QString(input.readAll()) + QStringLiteral("\n");
    input.close();

    QJSValue scriptFunction = m_pJSEngine->evaluate(scriptCode, filename);
    if (scriptFunction.isError()) {
        showScriptExceptionDialog(scriptFunction, true);
        return false;
    }

    return true;
}

QJSValue ControllerScriptEngineLegacy::wrapArrayBufferCallback(const QJSValue& callback) {
    return m_makeArrayBufferWrapperFunction.call(QJSValueList{callback});
}
