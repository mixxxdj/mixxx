#pragma once

#include <QObject>

#include "controllers/scripting/legacy/scriptconnection.h"

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
    Q_INVOKABLE virtual bool disconnect();
    Q_INVOKABLE virtual void trigger();

  private:
    QString m_idString;

  protected:
    ScriptConnection m_scriptConnection;
    bool m_isConnected;
};

/// ScriptRuntimeConnectionJSProxy provides scripts with an interface to
/// controller runtime update callback.
class ScriptRuntimeConnectionJSProxy : public ScriptConnectionJSProxy {
    Q_OBJECT
  public:
    ScriptRuntimeConnectionJSProxy(const ScriptConnection& conn)
            : ScriptConnectionJSProxy(conn) {
    }

    Q_INVOKABLE bool disconnect() override;
    Q_INVOKABLE void trigger() override;
};
