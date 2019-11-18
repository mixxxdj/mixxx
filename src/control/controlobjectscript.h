#ifndef CONTROLOBJECTSCRIPT_H
#define CONTROLOBJECTSCRIPT_H

#include <QVector>

#include "controllers/controllerengine.h"
#include "controllers/controllerdebug.h"
#include "control/controlproxy.h"

// this is used for communicate with controller scripts
class ControlObjectScript : public ControlProxy {
    Q_OBJECT
  public:
    explicit ControlObjectScript(const ConfigKey& key, QObject* pParent = nullptr);

    bool addScriptConnection(const ScriptConnection& conn);

    bool removeScriptConnection(const ScriptConnection& conn);

    // Required for legacy behavior of ControllerEngine::connectControl
    inline int countConnections() {
            return m_scriptConnections.size(); };
    inline ScriptConnection firstConnection() {
            return m_scriptConnections.first(); };
    void disconnectAllConnectionsToFunction(const QScriptValue& function);

    // Called from update();
    void emitValueChanged() override {
        emit(trigger(get(), this));
    }

  signals:
    // It will connect to the slotValueChanged as well
    void trigger(double, QObject*);

  protected slots:
    // Receives the value from the master control by a unique queued connection
    void slotValueChanged(double v, QObject*);

  private:
    QList<ScriptConnection> m_scriptConnections;
};

#endif // CONTROLOBJECTSCRIPT_H
