#pragma once

#include <memory>
#include <vector>

#include "controllers/controllerenumerator.h"

class Controller;

/// This class handles discovery and enumeration of DJ controllers that use the
/// USB-HID protocol.
class HidEnumerator : public ControllerEnumerator {
    Q_OBJECT
  public:
    HidEnumerator() = default;
    ~HidEnumerator() override;

    QList<Controller*> queryDevices() override;

  private:
    std::vector<std::unique_ptr<Controller>> m_devices;
};
