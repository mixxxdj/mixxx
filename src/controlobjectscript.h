#ifndef CONTROLOBJECTSCRIPT_H
#define CONTROLOBJECTSCRIPT_H

#include "controlobjectslave.h"

// this is used for communicate with controller scripts
class ControlObjectScript : public ControlObjectSlave {
    Q_OBJECT
  public:
    ControlObjectScript(const QString& g, const QString& i, QObject* pParent = NULL);
    virtual ~ControlObjectScript();

  protected slots:
    // Receives the value from the master control by a unique direct connection
    virtual void slotValueChangedDirect(double v, QObject* pSetter) {
        Q_UNUSED(pSetter) // we emit updates also if we are the setter
        emit(valueChanged(v));
    }

    // Receives the value from the master control by a unique auto connection
    virtual void slotValueChangedAuto(double v, QObject* pSetter) {
        Q_UNUSED(pSetter) // we emit updates also if we are the setter
        emit(valueChanged(v));
    }
};

#endif // CONTROLOBJECTSLAVE_H
