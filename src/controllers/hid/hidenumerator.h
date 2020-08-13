#ifndef HIDENUMERATOR_H
#define HIDENUMERATOR_H

#include "controllers/controllerenumerator.h"

/// This class handles discovery and enumeration of DJ controllers that use the
/// USB-HID protocol.
class HidEnumerator : public ControllerEnumerator {
  public:
    explicit HidEnumerator(UserSettingsPointer pConfig);
    virtual ~HidEnumerator();

    QList<Controller*> queryDevices();

  private:
    QList<Controller*> m_devices;
    UserSettingsPointer m_pConfig;
};

#endif
