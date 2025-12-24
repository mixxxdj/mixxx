#pragma once

#include <QObject>
#include <utility>

#include "controllers/scripting/legacy/scriptconnection.h"

/// ScriptConnectionJSProxy provides scripts with an interface to ScriptConnection.
class ScriptConnectionJSProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString id READ readId)
    Q_PROPERTY(bool isConnected READ readIsConnected)
  public:
    ScriptConnectionJSProxy(ScriptConnection conn)
            : m_scriptConnection(std::move(conn)),
              m_isConnected(true) {
    }
    QString readId() const {
        return m_scriptConnection.id.toString();
    }
    bool readIsConnected() const {
        return m_isConnected;
    }
    Q_INVOKABLE bool disconnect();
    Q_INVOKABLE void trigger();

  private:
    ScriptConnection m_scriptConnection;
    bool m_isConnected;
};
