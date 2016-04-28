#include <QApplication>
#include <QtDebug>

#include "control/controlobjectscript.h"

ControlObjectScript::ControlObjectScript(const ConfigKey& key, QObject* pParent)
        : ControlProxy(key, pParent) {
}

void ControlObjectScript::connectScriptFunction(
        const ControllerEngineConnection& conn) {
    if (m_connectedScriptFunctions.isEmpty()) {
        // we connect the slots only, if there will be actually a script
        // connected
        connect(m_pControl.data(), SIGNAL(valueChanged(double, QObject*)),
                this, SLOT(slotValueChanged(double,QObject*)),
                Qt::QueuedConnection);
        connect(this, SIGNAL(trigger(double, QObject*)),
                this, SLOT(slotValueChanged(double,QObject*)),
                Qt::QueuedConnection);
    }
    m_connectedScriptFunctions.append(conn);
}

bool ControlObjectScript::disconnectScriptFunction(
        const ControllerEngineConnection& conn) {
    bool ret = m_connectedScriptFunctions.removeAll(conn) > 0;
    if (m_connectedScriptFunctions.isEmpty()) {
        // no script left, we can disconnected
        disconnect(m_pControl.data(), SIGNAL(valueChanged(double, QObject*)),
                this, SLOT(slotValueChanged(double,QObject*)));
        disconnect(this, SIGNAL(trigger(double, QObject*)),
                this, SLOT(slotValueChanged(double,QObject*)));
    }
    return ret;
}

void ControlObjectScript::slotValueChanged(double value, QObject*) {
    // Make a local copy of m_connectedScriptFunctions fist.
    // This allows a script to disconnect a callback from the callback
    // itself. Otherwise the this may crash since the disconnect call
    // happens during conn.function.call() in the middle of the loop below.
    QList<ControllerEngineConnection> connections = m_connectedScriptFunctions;
    for(auto&& conn: connections) {
        QScriptValueList args;
        args << QScriptValue(value);
        args << QScriptValue(getKey().group);
        args << QScriptValue(getKey().item);
        QScriptValue result = conn.function.call(conn.context, args);
        if (result.isError()) {
            qWarning() << "ControllerEngine: Invocation of callback" << conn.id
                       << "failed:" << result.toString();
        }
    }
}
