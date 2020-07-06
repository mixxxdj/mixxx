#include "controllers/engine/scriptconnectionjsproxy.h"

#include "controllers/engine/controllerenginejsproxy.h"

bool ScriptConnectionJSProxy::disconnect() {
    // if the removeScriptConnection succeeded, the connection has been successfully disconnected
    bool success = m_scriptConnection.engineJSProxy->removeScriptConnection(m_scriptConnection);
    m_isConnected = !success;
    return success;
}

void ScriptConnectionJSProxy::trigger() {
    m_scriptConnection.engineJSProxy->triggerScriptConnection(m_scriptConnection);
}
