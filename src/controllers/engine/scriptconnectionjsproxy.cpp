#include "controllers/engine/scriptconnectionjsproxy.h"

#include "controllers/engine/controllerengine.h"
#include "moc_scriptconnectionjsproxy.cpp"

bool ScriptConnectionJSProxy::disconnect() {
    // if the removeScriptConnection succeeded, the connection has been successfully disconnected
    bool success = m_scriptConnection.controllerEngine->removeScriptConnection(m_scriptConnection);
    m_isConnected = !success;
    return success;
}

void ScriptConnectionJSProxy::trigger() {
    m_scriptConnection.controllerEngine->triggerScriptConnection(m_scriptConnection);
}
