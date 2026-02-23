#pragma once

#include <QJSValue>
#include <QUuid>

class ControllerScriptEngineLegacy;
class ControllerScriptInterfaceLegacy;

/// SharedDataConnection represents a subscription from a controller script
/// to changes of a specific entity/key pair in the shared data store.
/// The callback receives (value, entity, key) when the data changes.
class SharedDataConnection {
  public:
    QString entity;
    QString key;
    QUuid id;
    QJSValue callback;
    ControllerScriptInterfaceLegacy* engineJSProxy;
    ControllerScriptEngineLegacy* controllerEngine;

    void executeCallback(const QJSValue& value) const;

    inline bool operator==(const SharedDataConnection& other) const {
        return id == other.id;
    }
    inline bool operator!=(const SharedDataConnection& other) const {
        return !(*this == other);
    }
};
