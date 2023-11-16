#include "controllers/scripting/controllerscriptmoduleengine.h"

#include <QJSEngine>

#include "moc_controllerscriptmoduleengine.cpp"

ControllerScriptModuleEngine::ControllerScriptModuleEngine(
        Controller* controller, const RuntimeLoggingCategory& logger)
        : ControllerScriptEngineBase(controller, logger) {
    connect(&m_fileWatcher,
            &QFileSystemWatcher::fileChanged,
            this,
            &ControllerScriptModuleEngine::reload);
}

ControllerScriptModuleEngine::~ControllerScriptModuleEngine() {
    shutdown();
}

bool ControllerScriptModuleEngine::initialize() {
    ControllerScriptEngineBase::initialize();
    // TODO: Add new ControlObject JS API to scripting environment.

    QJSValue mod =
            m_pJSEngine->importModule(m_moduleFileInfo.absoluteFilePath());
    if (mod.isError()) {
        showScriptExceptionDialog(mod);
        shutdown();
        return false;
    }

    if (!m_fileWatcher.addPath(m_moduleFileInfo.absoluteFilePath())) {
        qCWarning(m_logger) << "Failed to watch script file" << m_moduleFileInfo.absoluteFilePath();
    }

    QJSValue initFunction = mod.property("init");
    if (!executeFunction(&initFunction)) {
        shutdown();
        return false;
    }

    QJSValue shutdownFunction = mod.property("shutdown");
    if (shutdownFunction.isCallable()) {
        m_shutdownFunction = shutdownFunction;
    } else {
        qCDebug(m_logger) << "Module exports no shutdown function.";
    }
    return true;
}

void ControllerScriptModuleEngine::shutdown() {
    executeFunction(&m_shutdownFunction);
    ControllerScriptEngineBase::shutdown();
}
