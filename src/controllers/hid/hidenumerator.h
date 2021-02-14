#pragma once

#include "controllers/controllerenumerator.h"

/// This class handles discovery and enumeration of DJ controllers that use the
/// USB-HID protocol.
class HidEnumerator : public ControllerEnumerator {
  public:
    HidEnumerator() = default;
    ~HidEnumerator() override;

    QList<Controller*> queryDevices() override;

  private:
    QList<Controller*> m_devices;
};
