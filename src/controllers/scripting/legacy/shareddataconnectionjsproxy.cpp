#include "controllers/scripting/legacy/shareddataconnectionjsproxy.h"

#include "controllers/scripting/legacy/controllerscriptinterfacelegacy.h"
#include "moc_shareddataconnectionjsproxy.cpp"

void SharedDataConnectionJSProxy::disconnect() {
    m_connection.engineJSProxy->removeSharedDataConnection(m_connection);
    m_isConnected = false;
}

void SharedDataConnectionJSProxy::trigger() {
    m_connection.engineJSProxy->triggerSharedDataConnection(m_connection);
}
