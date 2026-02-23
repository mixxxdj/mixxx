#include "controllers/scripting/legacy/shareddataconnectionjsproxy.h"

#include "controllers/scripting/legacy/controllerscriptinterfacelegacy.h"
#include "moc_shareddataconnectionjsproxy.cpp"

bool SharedDataConnectionJSProxy::disconnect() {
    bool success = m_connection.engineJSProxy->removeSharedDataConnection(
            m_connection);
    m_isConnected = !success;
    return success;
}

void SharedDataConnectionJSProxy::trigger() {
    m_connection.engineJSProxy->triggerSharedDataConnection(m_connection);
}
