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

  protected slots:
    // Receives the value from the master control by a unique queued connection
    void slotValueChanged(double v, QObject*);

  private:
    QList<ControllerEngineConnection> m_connectedScriptFunction;
};

#endif // CONTROLOBJECTSLAVE_H
