#pragma once

#include <QJSValue>
#include <QUuid>

#include "preferences/configobject.h"

class ControllerScriptEngineLegacy;
class ControllerScriptInterfaceLegacy;

/// ScriptConnection is a connection between a ControlObject and a
/// script callback function that gets executed when the value
/// of the ControlObject changes.
class ScriptConnection {
  public:
    ConfigKey key;
    QUuid id;
    QJSValue callback;
    ControllerScriptInterfaceLegacy* engineJSProxy;
    ControllerScriptEngineLegacy* controllerEngine;
    bool skipSuperseded;
    bool m_callbackExecuting;
    bool m_preventRecursiveCalls; // New flag to prevent recursive calls without a warning

    ScriptConnection();

    void executeCallback(double value);

    // Required for various QList methods and iteration to work.
    inline bool operator==(const ScriptConnection& other) const {
        return id == other.id;
    }
    inline bool operator!=(const ScriptConnection& other) const {
        return !(*this == other);
    }
    private:
    const RuntimeLoggingCategory m_logger;
};
