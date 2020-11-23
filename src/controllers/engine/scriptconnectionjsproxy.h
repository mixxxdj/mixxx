#pragma once

#include <QObject>

#include "controllers/engine/scriptconnection.h"

/// ScriptConnectionJSProxy provides scripts with an interface to ScriptConnection.
class ScriptConnectionJSProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString id READ readId)
    Q_PROPERTY(bool isConnected READ readIsConnected)
  public:
    ScriptConnectionJSProxy(const ScriptConnection& conn) {
        m_scriptConnection = conn;
        m_idString = conn.id.toString();
        m_isConnected = true;
    }
    const QString& readId() const {
        return m_idString;
    }
    bool readIsConnected() const {
        return m_isConnected;
    }
    Q_INVOKABLE bool disconnect();
    Q_INVOKABLE void trigger();

  private:
    ScriptConnection m_scriptConnection;
    QString m_idString;
    bool m_isConnected;
};
