#ifndef CONTROLOBJECTSCRIPT_H
#define CONTROLOBJECTSCRIPT_H

#include "controllers/controllerengine.h"

#include "controlobjectslave.h"

// this is used for communicate with controller scripts
class ControlObjectScript : public ControlObjectSlave {
    Q_OBJECT
  public:
    ControlObjectScript(const ConfigKey& key, QObject* pParent = NULL);
    virtual ~ControlObjectScript();

    bool connectScriptFunction(
            const ControllerEngineConnection& conn);

    bool disconnectScriptFunction(
            const ControllerEngineConnection& conn);

    // Called from update();
    virtual void emitValueChanged() {
        emit(trigger(get(), this));
    }

  signals:
    // It will connect to the slotValueChanged as well
    void trigger(double, QObject*);

  protected slots:
    // Receives the value from the master control by a unique queued connection
    void slotValueChanged(double v, QObject*);

  private:
    QList<ControllerEngineConnection> m_connectedScriptFunction;
};

#endif // CONTROLOBJECTSLAVE_H
