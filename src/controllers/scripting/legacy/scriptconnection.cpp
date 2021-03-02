#include "controllers/scripting/legacy/scriptconnection.h"

#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"

void ScriptConnection::executeCallback(double value) const {
    QJSValueList args;
    args << QJSValue(value);
    args << QJSValue(key.group);
    args << QJSValue(key.item);
    QJSValue func = callback; // copy function because QJSValue::call is not const
    QJSValue result = func.call(args);
    if (result.isError()) {
        if (controllerEngine != nullptr) {
            controllerEngine->showScriptExceptionDialog(result);
        }
        qWarning() << "ControllerEngine: Invocation of connection " << id.toString()
                   << "connected to (" + key.group + ", " + key.item + ") failed:"
                   << result.toString();
    }
}
