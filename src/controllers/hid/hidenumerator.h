#pragma once

#include "controllers/controllerenumerator.h"
#include "controllers/hid/hiddevice.h"

/// This class handles discovery and enumeration of DJ controllers that use the
/// USB-HID protocol.
class HidEnumerator : public ControllerEnumerator {
    Q_OBJECT
  public:
    bool recognizeDevice(const hid_device_info& device_info) const;
    HidEnumerator() = default;
    ~HidEnumerator() override;

    QList<Controller*> queryDevices() override;

  private:
    QList<Controller*> m_devices;
};
