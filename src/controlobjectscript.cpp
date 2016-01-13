#include <QApplication>
#include <QtDebug>

#include "controlobjectscript.h"

ControlObjectScript::ControlObjectScript(const ConfigKey& key, QObject* pParent)
        : ControlObjectSlave(key, pParent) {
}

void ControlObjectScript::connectScriptFunction(
        const ControllerEngineConnection& conn) {
    m_connectedScriptFunctions.append(conn);
    connect(m_pControl.data(), SIGNAL(valueChanged(double, QObject*)),
            this, SLOT(slotValueChanged(double,QObject*)),
            static_cast<Qt::ConnectionType>(Qt::QueuedConnection |
                                            Qt::UniqueConnection));
    connect(this, SIGNAL(trigger(double, QObject*)),
            this, SLOT(slotValueChanged(double,QObject*)),
            static_cast<Qt::ConnectionType>(Qt::QueuedConnection |
                                            Qt::UniqueConnection));
}

bool ControlObjectScript::disconnectScriptFunction(
        const ControllerEngineConnection& conn) {
    return m_connectedScriptFunctions.removeAll(conn) > 0;
}

void ControlObjectScript::slotValueChanged(double value, QObject*) {
    for(auto conn: m_connectedScriptFunctions) {
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
