#ifndef CONTROLOBJECTSCRIPT_H
#define CONTROLOBJECTSCRIPT_H

#include "controllers/controllerengine.h"
#include "controllers/controllerdebug.h"
#include "control/controlproxy.h"

// this is used for communicate with controller scripts
class ControlObjectScript : public ControlProxy {
    Q_OBJECT
  public:
    explicit ControlObjectScript(const ConfigKey& key, QObject* pParent = nullptr);

    bool addConnection(
            const ControllerEngineConnection& conn);

    bool removeConnection(
            const ControllerEngineConnection& conn);

    inline int countConnections() {
            return m_controllerEngineConnections.size(); };
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
    QList<ControllerEngineConnection> m_controllerEngineConnections;
};

#endif // CONTROLOBJECTSCRIPT_H
