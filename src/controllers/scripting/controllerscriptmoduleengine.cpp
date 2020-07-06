#include "controllers/scripting/controllerscriptmoduleengine.h"

bool ControllerScriptModuleEngine::initialize() {
    ControllerScriptEngineBase::initialize();
    m_pJSEngine->installExtensions(QJSEngine::ConsoleExtension);
    // TODO: Add new ControlObject JS API to scripting environment.

    QJSValue mod =
            m_pJSEngine->importModule(m_moduleFileInfo.absoluteFilePath());
    if (mod.isError()) {
        showScriptExceptionDialog(mod);
        shutdown();
        return false;
    }

    m_scriptWatcher.addPath(m_moduleFileInfo.absoluteFilePath());

    QJSValue initFunction = mod.property("init");
    if (!executeFunction(initFunction, QJSValueList{})) {
        shutdown();
        return false;
    }

    QJSValue handleInputFunction = mod.property("handleInput");
    if (handleInputFunction.isCallable()) {
        m_handleInputFunction = handleInputFunction;
    } else {
        scriptErrorDialog(
                "Controller JavaScript module exports no handleInput function.",
                QStringLiteral("handleInput"),
                true);
        shutdown();
        return false;
    }

    QJSValue shutdownFunction = mod.property("shutdown");
    if (shutdownFunction.isCallable()) {
        m_shutdownFunction = shutdownFunction;
    } else {
        qDebug() << "Module exports no shutdown function.";
    }
    return true;
}

void ControllerScriptModuleEngine::shutdown() {
    executeFunction(m_shutdownFunction, QJSValueList());
    ControllerScriptEngineBase::shutdown();
}

void ControllerScriptModuleEngine::handleInput(
        QByteArray data, mixxx::Duration timestamp) {
    QJSValueList args;
    args << byteArrayToScriptValue(data);
    args << timestamp.toDoubleMillis();
    executeFunction(m_handleInputFunction, args);
}
