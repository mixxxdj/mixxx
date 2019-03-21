#include <QtDebug>

#include "control/controlobjectscript.h"

ControlObjectScript::ControlObjectScript(const ConfigKey& key, QObject* pParent)
        : ControlProxy(key, pParent) {
}

bool ControlObjectScript::addScriptConnection(const ScriptConnection& conn) {
    if (m_scriptConnections.isEmpty()) {
        // Only connect the slots when they are actually needed
        // by script connections.
        connect(m_pControl.data(), SIGNAL(valueChanged(double, QObject*)),
                this, SLOT(slotValueChanged(double,QObject*)),
                Qt::QueuedConnection);
        connect(this, SIGNAL(trigger(double, QObject*)),
                this, SLOT(slotValueChanged(double,QObject*)),
                Qt::QueuedConnection);
    }

    for (const auto& priorConnection: m_scriptConnections) {
        if (conn == priorConnection) {
            qWarning() << "Connection " + conn.id.toString() +
                          " already connected to (" +
                          conn.key.group + ", " + conn.key.item +
                          "). Ignoring attempt to connect again.";
            return false;
        }
    }

    m_scriptConnections.append(conn);
    controllerDebug("Connected (" +
                    conn.key.group + ", " + conn.key.item +
                    ") to connection " + conn.id.toString());
    return true;
}

void ControlObjectScript::removeScriptConnection(const ScriptConnection& conn) {
    bool success = m_scriptConnections.removeOne(conn);
    if (success) {
        controllerDebug("Disconnected (" +
                        conn.key.group + ", " + conn.key.item +
                        ") from connection " + conn.id.toString());
    } else {
        qWarning() << "Failed to disconnect (" +
                      conn.key.group + ", " + conn.key.item +
                      ") from connection " + conn.id.toString();
    }
    if (m_scriptConnections.isEmpty()) {
        // no ScriptConnections left, so disconnect signals
        disconnect(m_pControl.data(), SIGNAL(valueChanged(double, QObject*)),
                this, SLOT(slotValueChanged(double,QObject*)));
        disconnect(this, SIGNAL(trigger(double, QObject*)),
                this, SLOT(slotValueChanged(double,QObject*)));
    }
}

void ControlObjectScript::disconnectAllConnectionsToFunction(const QScriptValue& function) {
    // Make a local copy of m_scriptConnections because items are removed within the loop.
    const QList<ScriptConnection> connections = m_scriptConnections;
    for (const auto& conn: connections) {
        if (conn.callback.strictlyEquals(function)) {
            removeScriptConnection(conn);
        }
    }
}

void ControlObjectScript::slotValueChanged(double value, QObject*) {
    // Make a local copy of m_connectedScriptFunctions first.
    // This allows a script to disconnect a callback from inside the
    // the callback. Otherwise the this may crash since the disconnect call
    // happens during conn.function.call() in the middle of the loop below.
    const QList<ScriptConnection> connections = m_scriptConnections;
    for (auto&& conn: connections) {
        conn.executeCallback(value);
    }
}
