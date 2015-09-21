#include <QApplication>
#include <QtDebug>

#include "controlobjectscript.h"

ControlObjectScript::ControlObjectScript(const ConfigKey& key, QObject* pParent)
        : ControlObjectSlave(key, pParent) {
}

ControlObjectScript::~ControlObjectScript() {
    //qDebug() << "ControlObjectScript::~ControlObjectSlave()";
}

bool ControlObjectScript::connectScriptFunction(
        const ControllerEngineConnection& conn) {
    m_connectedScriptFunction.append(conn);
    connect(m_pControl.data(), SIGNAL(valueChanged(double, QObject*)),
            this, SLOT(slotValueChanged(double,QObject*)),
            static_cast<Qt::ConnectionType>(Qt::QueuedConnection |
                                            Qt::UniqueConnection));
    return true;
}

bool ControlObjectScript::disconnectScriptFunction(
        const ControllerEngineConnection& conn) {
    return m_connectedScriptFunction.removeAll(conn) > 0;
}

void ControlObjectScript::slotValueChanged(double value, QObject*) {
    foreach(ControllerEngineConnection conn, m_connectedScriptFunction) {
        QScriptValueList args;
        args << QScriptValue(value);
        args << QScriptValue(getKey().group);
        args << QScriptValue(getKey().item);
        QScriptValue result = conn.function.call(conn.context, args);
        if (result.isError()) {
            qWarning()<< "ControllerEngine: Call to callback" << conn.id
                      << "resulted in an error:" << result.toString();
        }
    }
}
