#include "control/controlobjectscript.h"

#include "moc_controlobjectscript.cpp"

ControlObjectScript::ControlObjectScript(
        const ConfigKey& key, const RuntimeLoggingCategory& logger, QObject* pParent)
        : ControlProxy(key, pParent, ControlFlag::AllowMissingOrInvalid),
          m_logger(logger),
          m_proxy(key, logger, this),
          m_skipSuperseded(false) {
}

bool ControlObjectScript::addScriptConnection(const ScriptConnection& conn) {
    if (m_scriptConnections.isEmpty()) {
        // Only connect the slots when they are actually needed
        // by script connections.
        m_skipSuperseded = conn.skipSuperseded;
        if (conn.skipSuperseded) {
            connect(m_pControl.data(),
                    &ControlDoublePrivate::valueChanged,
                    &m_proxy,
                    &CompressingProxy::slotValueChanged,
                    Qt::QueuedConnection);
            connect(&m_proxy,
                    &CompressingProxy::signalValueChanged,
                    this,
                    &ControlObjectScript::slotValueChanged,
                    Qt::DirectConnection);
        } else {
            connect(m_pControl.data(),
                    &ControlDoublePrivate::valueChanged,
                    this,
                    &ControlObjectScript::slotValueChanged,
                    Qt::QueuedConnection);
        }
        connect(this,
                &ControlObjectScript::trigger,
                this,
                &ControlObjectScript::slotValueChanged,
                Qt::QueuedConnection);
    } else {
        // At least one callback function is already connected to this CO
        if (conn.skipSuperseded == false && m_skipSuperseded == true) {
            // Disconnect proxy if this is first callback function connected with skipSuperseded false
            m_skipSuperseded = false;
            qCWarning(m_logger) << conn.key.group + ", " + conn.key.item +
                            "is connected to different callback functions with "
                            "differing state of the skipSuperseded. Disable "
                            "skipping of superseded events for all these "
                            "callback functions.";
            disconnect(m_pControl.data(),
                    &ControlDoublePrivate::valueChanged,
                    &m_proxy,
                    &CompressingProxy::slotValueChanged);
            disconnect(&m_proxy,
                    &CompressingProxy::signalValueChanged,
                    this,
                    &ControlObjectScript::slotValueChanged);
            connect(m_pControl.data(),
                    &ControlDoublePrivate::valueChanged,
                    this,
                    &ControlObjectScript::slotValueChanged,
                    Qt::QueuedConnection);
        }
    }

    for (const auto& priorConnection : std::as_const(m_scriptConnections)) {
        if (conn == priorConnection) {
            qCWarning(m_logger) << "Connection " + conn.id.toString() +
                            " already connected to (" +
                            conn.key.group + ", " + conn.key.item +
                            "). Ignoring attempt to connect again.";
            return false;
        }
    }

    m_scriptConnections.append(conn);
    qCDebug(m_logger) << "Connected (" +
                    conn.key.group + ", " + conn.key.item +
                    ") to connection " + conn.id.toString();
    return true;
}

bool ControlObjectScript::removeScriptConnection(const ScriptConnection& conn) {
    bool success = m_scriptConnections.removeOne(conn);
    if (success) {
        qCDebug(m_logger) << "Disconnected (" +
                        conn.key.group + ", " + conn.key.item +
                        ") from connection " + conn.id.toString();
    } else {
        qCWarning(m_logger) << "Failed to disconnect (" +
                        conn.key.group + ", " + conn.key.item +
                        ") from connection " + conn.id.toString();
    }
    if (m_scriptConnections.isEmpty()) {
        // no ScriptConnections left, so disconnect signals
        if (m_skipSuperseded) {
            disconnect(m_pControl.data(),
                    &ControlDoublePrivate::valueChanged,
                    &m_proxy,
                    &CompressingProxy::slotValueChanged);
            disconnect(&m_proxy,
                    &CompressingProxy::signalValueChanged,
                    this,
                    &ControlObjectScript::slotValueChanged);
        } else {
            disconnect(m_pControl.data(),
                    &ControlDoublePrivate::valueChanged,
                    this,
                    &ControlObjectScript::slotValueChanged);
        }
        disconnect(this,
                &ControlObjectScript::trigger,
                this,
                &ControlObjectScript::slotValueChanged);
    }
    return success;
}

void ControlObjectScript::disconnectAllConnectionsToFunction(const QJSValue& function) {
    // Make a local copy of m_scriptConnections because items are removed within the loop.
    const QVector<ScriptConnection> connections = m_scriptConnections;
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
    const QVector<ScriptConnection> connections = m_scriptConnections;

    for (auto&& conn: connections) {
          conn.executeCallback(value);
    }
}
