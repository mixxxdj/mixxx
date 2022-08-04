#include "controllers/scripting/legacy/scriptconnection.h"

#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"
#include "util/trace.h"

void ScriptConnection::executeCallback(double value) const {
    Trace executeCallbackTrace("JS %1 callback", key.item);
    const auto args = QJSValueList{
            value,
            key.group,
            key.item,
    };
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
