#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"

#include "control/controlobject.h"
#include "controllers/controller.h"
#include "controllers/controllerdebug.h"
#include "controllers/scripting/colormapperjsproxy.h"
#include "controllers/scripting/legacy/controllerscriptinterfacelegacy.h"
#include "errordialoghandler.h"
#include "mixer/playermanager.h"

ControllerScriptEngineLegacy::ControllerScriptEngineLegacy(Controller* controller)
        : ControllerScriptEngineBase(controller) {
    connect(&m_fileWatcher,
            &QFileSystemWatcher::fileChanged,
            this,
            &ControllerScriptEngineLegacy::reload);
}

ControllerScriptEngineLegacy::~ControllerScriptEngineLegacy() {
    shutdown();
}

bool ControllerScriptEngineLegacy::callFunctionOnObjects(QList<QString> scriptFunctionPrefixes,
        const QString& function,
        QJSValueList args,
        bool bFatalError) {
    VERIFY_OR_DEBUG_ASSERT(m_pJSEngine) {
        return false;
    }

    const QJSValue global = m_pJSEngine->globalObject();

    bool success = true;
    for (const QString& prefixName : scriptFunctionPrefixes) {
        QJSValue prefix = global.property(prefixName);
        if (!prefix.isObject()) {
            qWarning() << "No" << prefixName << "object in script";
            continue;
        }

        QJSValue init = prefix.property(function);
        if (!init.isCallable()) {
            qWarning() << prefixName << "has no"
                       << function << " method";
            continue;
        }
        controllerDebug("Executing"
                << prefixName << "." << function);
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

bool ControllerScriptEngineLegacy::initialize() {
    ControllerScriptEngineBase::initialize();

    // Make this ControllerScriptHandler instance available to scripts as 'engine'.
    QJSValue engineGlobalObject = m_pJSEngine->globalObject();
    ControllerScriptInterfaceLegacy* legacyScriptInterface =
            new ControllerScriptInterfaceLegacy(this);
    engineGlobalObject.setProperty(
            "engine", m_pJSEngine->newQObject(legacyScriptInterface));

    for (const ControllerPreset::ScriptFileInfo& script : m_scriptFiles) {
        if (!evaluateScriptFile(script.file)) {
            shutdown();
            return false;
        }
        if (!script.functionPrefix.isEmpty()) {
            m_scriptFunctionPrefixes.append(script.functionPrefix);
        }
    }

    for (QString functionName : m_scriptFunctionPrefixes) {
        if (functionName.isEmpty()) {
            continue;
        }
        functionName.append(QStringLiteral(".incomingData"));
        m_incomingDataFunctions.append(
                wrapArrayBufferCallback(
                        wrapFunctionCode(functionName, 2)));
    }

    QJSValueList args;
    if (m_pController) {
        args << QJSValue(m_pController->getName());
    } else { // m_pController is nullptr in tests.
        args << QJSValue();
    }
    args << QJSValue(ControllerDebug::enabled());
    if (!callFunctionOnObjects(m_scriptFunctionPrefixes, "init", args, true)) {
        shutdown();
        return false;
    }

    return true;
}

void ControllerScriptEngineLegacy::shutdown() {
    callFunctionOnObjects(m_scriptFunctionPrefixes, "shutdown");
    m_scriptWrappedFunctionCache.clear();
    m_incomingDataFunctions.clear();
    m_scriptFunctionPrefixes.clear();
    ControllerScriptEngineBase::shutdown();
}

bool ControllerScriptEngineLegacy::handleIncomingData(const QByteArray& data) {
    // This function is called from outside the controller engine, so we can't
    // use VERIFY_OR_DEBUG_ASSERT here
    if (!m_pJSEngine) {
        return false;
    }

    QJSValueList args;
    args << m_pJSEngine->toScriptValue(data);
    args << QJSValue(data.size());

    for (const QJSValue& function : m_incomingDataFunctions) {
        ControllerScriptEngineBase::executeFunction(function, args);
    }

    return true;
}

bool ControllerScriptEngineLegacy::evaluateScriptFile(const QFileInfo& scriptFile) {
    VERIFY_OR_DEBUG_ASSERT(m_pJSEngine) {
        return false;
    }

    if (!scriptFile.exists()) {
        qWarning() << "File does not exist:"
                   << scriptFile.absoluteFilePath();
        return false;
    }

    // If the script is invalid, it should be watched so the user can fix it
    // without having to restart Mixxx. So, add it to the watcher before
    // evaluating it.
    if (!m_fileWatcher.addPath(scriptFile.absoluteFilePath())) {
        qWarning() << "Failed to watch script file" << scriptFile.absoluteFilePath();
    };

    qDebug() << "Loading"
             << scriptFile.absoluteFilePath();

    // Read in the script file
    QString filename = scriptFile.absoluteFilePath();
    QFile input(filename);
    if (!input.open(QIODevice::ReadOnly)) {
        qWarning() << QString(
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
                input.errorString());

        // Ask above layer to display the dialog & handle user response
        ErrorDialogHandler::instance()->requestErrorDialog(props);
        return false;
    }

    QString scriptCode;
    scriptCode.append(input.readAll());
    scriptCode.append('\n');
    input.close();

    QJSValue scriptFunction = m_pJSEngine->evaluate(scriptCode, filename);
    if (scriptFunction.isError()) {
        showScriptExceptionDialog(scriptFunction, true);
        return false;
    }

    return true;
}
