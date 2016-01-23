#ifndef CONTROLOBJECTSCRIPT_H
#define CONTROLOBJECTSCRIPT_H

#include "controllers/controllerengine.h"

#include "controlobjectslave.h"

// this is used for communicate with controller scripts
class ControlObjectScript : public ControlObjectSlave {
    Q_OBJECT
  public:
    explicit ControlObjectScript(const ConfigKey& key, QObject* pParent = nullptr);

    void connectScriptFunction(
            const ControllerEngineConnection& conn);

    bool disconnectScriptFunction(
            const ControllerEngineConnection& conn);

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
    QList<ControllerEngineConnection> m_connectedScriptFunctions;
};

#endif // CONTROLOBJECTSLAVE_H
