#pragma once

#include <QObject>
#include <utility>

#include "controllers/scripting/legacy/shareddataconnection.h"

/// SharedDataConnectionJSProxy provides scripts with an interface to
/// SharedDataConnection — the connection between a shared data entity/key
/// and a JavaScript callback function.
class SharedDataConnectionJSProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString id READ readId)
    Q_PROPERTY(bool isConnected READ readIsConnected)
  public:
    SharedDataConnectionJSProxy(SharedDataConnection conn)
            : m_connection(std::move(conn)),
              m_isConnected(true) {
    }
    QString readId() const {
        return m_connection.id.toString();
    }
    bool readIsConnected() const {
        return m_isConnected;
    }
    Q_INVOKABLE void disconnect();
    Q_INVOKABLE void trigger();

  private:
    SharedDataConnection m_connection;
    bool m_isConnected;
};
