#include "controllers/scripting/legacy/shareddataconnection.h"

#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"

void SharedDataConnection::executeCallback(const QJSValue& value) const {
    const auto args = QJSValueList{
            value,
            QJSValue(entity),
            QJSValue(key),
    };
    QJSValue func = callback; // copy because QJSValue::call is not const
    QJSValue result = func.call(args);
    if (result.isError()) {
        if (controllerEngine != nullptr) {
            controllerEngine->showScriptExceptionDialog(result);
        }
        qWarning() << "ControllerEngine: Invocation of shared data connection"
                   << id.toString()
                   << "connected to (" + entity + ", " + key + ") failed:"
                   << result.toString();
    }
}
