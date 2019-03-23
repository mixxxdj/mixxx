#ifndef OSCENUMERATOR_H
#define OSCENUMERATOR_H

#include "controllers/controllerenumerator.h"
#include "controllers/osc/osccontroller.h"

class OscEnumerator : public ControllerEnumerator {
    Q_OBJECT
  public:
    OscEnumerator();
    ~OscEnumerator();

    QList<Controller*> queryDevices();

  private:
    QList<Controller*> m_devices;
};

#endif /* OSCENUMERATOR_H */
