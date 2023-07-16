#include "controllers/scripting/legacy/scriptconnection.h"

#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"
#include "util/trace.h"


ScriptConnection::ScriptConnection()
        : m_callbackExecuting(false),
          m_preventRecursiveCalls(false),
          m_logger(logger) {
}

void ScriptConnection::executeCallback(double value) {
    Trace executeCallbackTrace("JS %1 callback", key.item);
    if (!m_preventRecursiveCalls) {
        if (!m_callbackExecuting) {
            m_callbackExecuting = true;

            const auto args = QJSValueList{
                    value,
                    key.group,
                    key.item,
            };
            QJSValue func = callback; // copy function because QJSValue::call is not const
            QJSValue result = func.call(args);

            m_callbackExecuting = false;

            if (result.isError()) {
                if (controllerEngine != nullptr) {
                    controllerEngine->showScriptExceptionDialog(result);
                }
                qWarning() << "ControllerEngine: Invocation of connection " << id.toString()
                           << "connected to (" + key.group + ", " + key.item + ") failed:"
                           << result.toString();
            }

        } else {
            qCCritical(m_logger) << "Critical Error: Triggering a connection "
                                    "inside the callback is not allowed.";
        }
    }
}
