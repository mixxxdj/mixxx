#include <QtDebug>

#include "control/controlobjectscript.h"

ControlObjectScript::ControlObjectScript(const ConfigKey& key, QObject* pParent)
        : ControlProxy(key, pParent) {
}

bool ControlObjectScript::addConnection(
        const ControllerEngineConnection& conn) {
    if (m_controllerEngineConnections.isEmpty()) {
        // Only connect the slots when they are actually needed
        // by script connections.
        connect(m_pControl.data(), SIGNAL(valueChanged(double, QObject*)),
                this, SLOT(slotValueChanged(double,QObject*)),
                Qt::QueuedConnection);
        connect(this, SIGNAL(trigger(double, QObject*)),
                this, SLOT(slotValueChanged(double,QObject*)),
                Qt::QueuedConnection);
    }

    for (const auto& priorConnection: m_controllerEngineConnections) {
        if (conn == priorConnection) {
            qWarning() << "Connection " + conn.id.toString() +
                          " already connected to (" +
                          conn.key.group + ", " + conn.key.item +
                          "). Ignoring attempt to connect again.";
            return false;
        }
    }

    m_controllerEngineConnections.append(conn);
    controllerDebug("Connected (" +
                    conn.key.group + ", " + conn.key.item +
                    ") to connection " + conn.id.toString());
    return true;
}

bool ControlObjectScript::removeConnection(
        const ControllerEngineConnection& conn) {
    bool success = m_controllerEngineConnections.removeOne(conn);
    if (success) {
        controllerDebug("Disconnected (" +
                        conn.key.group + ", " + conn.key.item +
                        ") from connection " + conn.id.toString());
    } else {
        qWarning() << "Failed to disconnect (" +
                      conn.key.group + ", " + conn.key.item +
                      ") from connection " + conn.id.toString();
    }
    if (m_controllerEngineConnections.isEmpty()) {
        // no script left, we can disconnected
        disconnect(m_pControl.data(), SIGNAL(valueChanged(double, QObject*)),
                this, SLOT(slotValueChanged(double,QObject*)));
        disconnect(this, SIGNAL(trigger(double, QObject*)),
                this, SLOT(slotValueChanged(double,QObject*)));
    }
    return success;
}

void ControlObjectScript::disconnectAllConnectionsToFunction(const QScriptValue& function) {
    // Make a local copy of m_controllerEngineConnections because items are removed
    // within the loop.
    QList<ControllerEngineConnection> connections = m_controllerEngineConnections;
    for (const auto& conn: connections) {
        if (conn.function.strictlyEquals(function)) {
            m_controllerEngineConnections.removeOne(conn);
        }
    }
}

void ControlObjectScript::slotValueChanged(double value, QObject*) {
    // Make a local copy of m_connectedScriptFunctions first.
    // This allows a script to disconnect a callback from the callback
    // itself. Otherwise the this may crash since the disconnect call
    // happens during conn.function.call() in the middle of the loop below.
    QList<ControllerEngineConnection> connections = m_controllerEngineConnections;
    for (auto&& conn: connections) {
        conn.executeCallback(value);
    }
}
