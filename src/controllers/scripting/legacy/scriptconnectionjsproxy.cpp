#include "controllers/scripting/legacy/scriptconnectionjsproxy.h"

#include "controllers/scripting/legacy/controllerscriptinterfacelegacy.h"
#include "moc_scriptconnectionjsproxy.cpp"

bool ScriptConnectionJSProxy::disconnect() {
    // if the removeScriptConnection succeeded, the connection has been successfully disconnected
    bool success = m_scriptConnection.engineJSProxy->removeScriptConnection(m_scriptConnection);
    m_isConnected = !success;
    return success;
}

void ScriptConnectionJSProxy::trigger() {
    m_scriptConnection.engineJSProxy->triggerScriptConnection(m_scriptConnection);
}

bool ScriptRuntimeConnectionJSProxy::disconnect() {
    // if the removeRuntimeDataConnection succeeded, the connection has been
    // successfully disconnected
    bool success =
            m_scriptConnection.engineJSProxy->removeRuntimeDataConnection(
                    m_scriptConnection);
    m_isConnected = !success;
    return success;
}

void ScriptRuntimeConnectionJSProxy::trigger() {
    m_scriptConnection.engineJSProxy->triggerRuntimeDataConnection(m_scriptConnection);
}
